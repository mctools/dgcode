
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

from dgdepfixer.sysutils import print, AbsPath,AbsPathOrNone
Sys = DG.sysutils

class stdinstaller:
    __dirs_created = []#here for pylint


    #methods to override for a specific installation project:
    def name(self): raise NotImplementedError
    def default_version(self): raise NotImplementedError
    def download_url(self,version): raise NotImplementedError
    def unpack_command(self,version): return None#if unpacking is needed, this cmd must unpack to source/ subdir
    def src_work_subdir(self,version): return ''#after unpacking, is there a subdir to step into before proceeding with configuration?
    def in_source_buildandconfig(self): return True#normally True for autotools, False for CMake
    def configure_command(self,instdir,srcdir,blddir,version,extra_options): raise NotImplementedError
    def validate_version(self,version): raise NotImplementedError
    def build_command(self,nprocs): return 'make -j%i'%nprocs
    def install_command(self,nprocs): return 'make -j%i install'%nprocs
    def allow_local_source(self): return False
    def libdirs_in_installation(self): return []#provide to get fixed install_names on OSX automatically
    def local_source_to_version(self,srcfile_basename): raise NotImplementedError
    def setup_file_contents(self,instdir): raise NotImplementedError
    def prefix_env_var(self): return None#provide if an environment variable XXX points to the installation area
    def unsetup_file_contents(self,instdir): raise NotImplementedError
    def dbg_flags(self): raise NotImplementedError
    def hide_flags(self): return []
    def check_reqs(self): return None#Will be called after parsing settings and
                                     #before starting download. If something is
                                     #wrong (e.g. Geant4 installed with GDML but
                                     #XercesC is not found), write error
                                     #messages and raise SystemExit or RuntimeError.

    #helper methods which can be used in derived classes when composing flags for the configuration command:
    def _prune_duplicate_flags(self,flags):
        seen = set()
        out=[]
        for f in reversed(flags):
            if not '=' in f:
                out+=[f]
                continue
            name,val=f.split('=',1)
            if name:
                if name not in seen:
                    out+=[f]
                    seen.add(name)
        out.reverse()
        return out
    def add_parser_options(self,parser,isfullhelp,hidefullfct): return
    def get_parser_options(self,opt): return
    def get_autoname_extras(self): return ''
    def get_autoname_extras_descr(self): return ''


    def _get_active_dgdepdir(self):
        return AbsPathOrNone(os.environ.get('DGDEPDIR',None))

    def parse_cmdline(self):
        #Most users should run this with an active DGDEPDIR, and thus have the
        #default install location set somewhere where the sourcing of the
        #created setup.sh file will be automatic. The --dir option can of course
        #override this, and it must be possible to run without an active
        #DGDEPDIR (because we want to support running
        #.system/depfix/binextra/dgdepfixer_install_xxx directly from a dgcode
        #clone)

        dgdepdir = self._get_active_dgdepdir()
        dgdepdiroptinst = dgdepdir.joinpath('extras') if dgdepdir else None

        n=self.name()
        hide_flags = set(self.hide_flags())
        allowed_hide_flags=['--debug','--jobs','--nocleanup','<customoptions>']
        for hf in hide_flags:
            if not hf in allowed_hide_flags:
                raise RuntimeError('implementation error')

        hide_customopts = ('<customoptions>' in hide_flags)#FIXME: unused??

        if dgdepdir:
            descr = ('Script for making installation of %s easier. Simply run it to install %s into your active DGDEPDIR installation (at %s).'%(n,n,dgdepdir))
        else:
            descr = ('Script for making installation of %s easier.'
                     ' WARNING: The script is being run without an active DGDEPDIR'
                     ' installation. Thus, the user MUST specify a target destination'
                     ' using --dir, and after installation it is necessary to source the setup.sh file'
                     ' in the target directory in order to use %s.'%(n,n))

        parser = argparse.ArgumentParser(description=descr,
                                         usage='%%(prog)s [options]%s'%('' if '<customoptions>' in hide_flags
                                                                       else ' [-- <custom options for configuration step>]'))

        #Custom solution for hiding some options unless requested (Note that --fu*
        #is enough to select --fullhelp, but --f is not due to the chance of conflicting options:
        full_help = any(a.startswith('--fu') for a in sys.argv[1:])
        hide = (lambda msg : msg) if full_help else (lambda _ : argparse.SUPPRESS)
        extra_args = ['--help'] if full_help else []
        parser.add_argument('--fullhelp', action='store_true',help="""Show help for expert-only options.""")

        helptext_dir = "Install %s in DIR (if DIR contains the string '%%%%name' it will expand to %s-<version>[_dbg]%s)."%(n,n.lower(),self.get_autoname_extras_descr())
        if dgdepdir:
            helptext_dir += ' It is highly recommended to *NOT* use this flag to override the default location inside $DGDEPDIR, where it will be automatically found and enabled by $DGDEPDIR/setup.sh'

        if dgdepdir:
            helptext_dir = hide(helptext_dir)#expert-only if running from active DGDEPDIR
        parser.add_argument("-d", "--dir",metavar='DIR',type=AbsPathOrNone, dest="instdir", default=None,help=helptext_dir)

        if not '--jobs' in hide_flags:
            parser.add_argument("-j", "--jobs",metavar="N",
                              type=int, dest="nprocs", default=0,
                                help=hide("Use up to N parallel processes during the build (default: autodetect)"))
        if self.allow_local_source():
            parser.add_argument("--srcfile",metavar="SRCFILE",
                              type=AbsPathOrNone, dest="localsrc", default=None,
                              help=hide("Use previously downloaded SRCFILE rather than downloading a new one"))
        parser.add_argument("-v", "--version",metavar="VERSION",
                            type=str, dest="version", default=None,
                            help=hide("Version to install (default %s)"%self.default_version()))
        parser.add_argument("--tmpdir",default=None,metavar='DIR',type=AbsPathOrNone,dest="tmpworkdir",
                            help=hide('Set to specify alternative DIR which to use for all temporary build files. On network file-system it can occasionally speed up the build to use /tmp for heavy-duty work.'))
        if not '--nocleanup' in hide_flags:
            parser.add_argument("--nocleanup",default=False,action='store_true',dest="nocleanup",
                                help=hide('Set to prevent cleanup of temporary source and build files'))
        if not '--debug' in hide_flags:
            parser.add_argument("--debug",default=False,action='store_true',dest="dbginfo",
                                help=hide('Set to build libs with debug info and keep source files'
                                          +' around (for optimal usage of debugging tools like gdb and valgrind).'))
        self.add_parser_options(parser,full_help,hide)
        args,cfgargs=sys.argv[1:],[]
        if '--' in args and not '<customoptions>' in hide_flags:
            cfgargs = args[args.index('--')+1:]
            args = args[0:args.index('--')]
        opt = parser.parse_args(args+extra_args)
        if '--debug' in hide_flags: opt.dbginfo=False
        if '--nocleanup' in hide_flags: opt.nocleanup=False
        if '--jobs' in hide_flags: opt.nprocs=1
        #if args:
        #    parser.error("Unknown arguments: '%s'"%("', '".join(args)))
        if opt.dbginfo:
            cfgargs += self.dbg_flags()
        opt.version_set_by_user = (opt.version!=None)
        if not opt.version_set_by_user:
            opt.version = self.default_version()
        if opt.nprocs<0 or opt.nprocs>999:
            parser.error("Number of jobs argument out of range")
        if not self.validate_version(opt.version):
            parser.error("Invalid version format")
        if opt.localsrc:
            if not opt.localsrc.exists():
                parser.error('Specified local source file not found: %s'%opt.localsrc)
            if not opt.version_set_by_user:
                #must autodetect
                v=self.local_source_to_version(opt.localsrc.name)
                if not v or not self.validate_version(v):
                    parser.error('Could not extract version from name of local source (specify using'
                                 +' --version flag instead): %s'%opt.localsrc.name)
                #if opt.version!=v:
                #    parser.error('Version specified does not correspond to detected version of local source file')
                opt.version=v
                print('::: Notice: Guessing from filename that this is version "%s" (if incorrect, set with --version flag)'%v)
        #Expand any %name or ~ in opt.instdir
        autoname='%s-%s%s'%(n.lower(),opt.version,'_dbg' if opt.dbginfo else '')
        self.get_parser_options(opt)
        autoname += self.get_autoname_extras()
        if opt.instdir:
            if '%name' in str(opt.instdir):
                opt.instdir=AbsPath(str(opt.instdir).replace('%name',autoname))
        if not opt.instdir:
            if not dgdepdir:
                parser.error('Please specify destination directory using --dir=DIR (needed since the DGDEPDIR environment variable is not set)')
            if any(dgdepdiroptinst.glob('%s-*/setup.sh'%n.lower())) or any(dgdepdiroptinst.glob('%s_*/setup.sh'%n.lower())):
                #DGDEPDIR already contains one installation of this type -
                #there should be at most one, so the DGDEPDIR/setup.sh script
                #unambiguously knows which one to source.
                parser.error('There is already an existing %s installation inside %s. Either remove it or specify an alternative destination with the --dir=DIR flag'%(n,dgdepdiroptinst))
            opt.instdir = dgdepdiroptinst.joinpath(autoname)
        else:
            #Check that the user did not manually specify a location inside any dgdepfixer installation
            for p in opt.instdir.parents:
                if p.joinpath('.dgcode_dependencies_installation').exists():
                    parser.error('Do not manually specify a target location inside a dgdepfixer installation (one detected at %s).'+
                                 (' Leave the --dir/-d flag unset to install inside the currently active installation at %s'%dgdepdir if dgdepdir else ''))
        assert opt.instdir
        if not Sys.isemptydir(opt.instdir):
            if opt.instdir.exists():
                parser.error("Already exists and not an empty directory (you can manually remove it and try again): %s"%opt.instdir)
            #if not opt.instdir.parent.exists():
            #    parser.error("Not found: %s"%opt.instdir.parent)
            if opt.instdir.parent.exists() and not opt.instdir.parent.is_dir():
                parser.error("Not a directory: %s"%opt.instdir.parent)
        if opt.tmpworkdir and not Sys.isemptydir(opt.tmpworkdir):
            if opt.tmpworkdir.exists():
                parser.error("Already exists and not an empty directory (you can manually remove it and try again): %s"%opt.tmpworkdir)
            if not opt.tmpworkdir.parent.exists():
                parser.error("Not found: %s"%opt.tmpworkdir.parent)
            if not opt.tmpworkdir.parent.is_dir():
                parser.error("Not a directory: %s"%opt.tmpworkdir.parent)
        if ' ' in str(opt.instdir):
            parser.error('Destination directory path should not contain spaces')
        dgcode_dir=AbsPathOrNone(os.environ.get('DGCODE_DIR',None))
        if dgcode_dir:
            if dgcode_dir in opt.instdir.parents:
                parser.error('You should choose an installation directory outside dgcode')
        return opt,cfgargs

    def _content_setup_files(self,instdir):
        fluff='# Autogenerated file for %s installation created with %s'%(self.name(),os.path.basename(sys.argv[0]))
        fluff=['','#'*(len(fluff)+2),fluff+' #','#'*(len(fluff)+2),'']
        prunepath=['function prunepath() {',
                   '    P=$(IFS=:;for p in ${!1:-}; do [[ $p != ${2}* ]] && echo -n ":$p"; done)',
                   '    export $1=${P:1:99999}',
                   '}','']
        setup_cont=self.setup_file_contents(instdir)
        if setup_cont is None:
            return None,None#Disable setup.sh+unsetup.sh generation!
        pev=self.prefix_env_var()
        if pev:
            #if different installation was sourced, call it's unsetup.sh first:
            setup_cont=['if [ "x${%s}" != "x" -a -f "${%s}"/unsetup.sh ]; then'%(pev,pev),
                        '    . "${%s}"/unsetup.sh'%pev,'fi','']+setup_cont
        unsetup_cont=self.unsetup_file_contents(instdir)
        setup_cont = '\n'.join(['#!/bin/bash']+fluff+setup_cont+[''])
        unsetup_cont = '\n'.join(['#!/bin/bash']+fluff+prunepath+unsetup_cont+['','unset prunepath',''])
        return setup_cont,unsetup_cont

    def go(self):
        def flush():
            sys.stdout.flush()
            sys.stderr.flush()
        def cleanup_msg():
            leftovers = [d for d in self.__dirs_created if d.exists()]
            if leftovers:
                if len(leftovers)==1:
                    print("The following directory was created and is likely left with invalid content.")
                else:
                    print("The following directories were created and are likely left with invalid content.")
                print()
                for l in leftovers:
                    print('     %s'%l)
                print()
                print("You should manually cleanup your system.")
                print()
        try:
            self.actual_go()
        except KeyboardInterrupt:
            flush()
            time.sleep(1)
            print ()
            print ("ERROR: Installation aborted by user.")
            print ()
            cleanup_msg()
            sys.exit(1)
        except SystemExit as e:
            flush()
            if e.code==0:
                raise SystemExit(0)
            time.sleep(1)
            if not isinstance(e.code,int):
                print("Problem encountered: %s"%e)
            print ()
            print ("ERROR: Installation ended prematurely.")
            print ()
            cleanup_msg()
            flush()
            raise SystemExit(1)
        except Exception as e:
            flush()
            time.sleep(1)
            print ("Exception caught: ",str(e))
            print ()
            traceback.print_exc()
            print ()
            print ("ERROR: Installation ended prematurely.")
            print ()
            cleanup_msg()
            flush()
            raise SystemExit(1)

    def actual_go(self):
        self.__dirs_created = []#Dirs user should be adviced to cleanup in case of problems.
        opt,cfgargs = self.parse_cmdline()
        opt.instdir = opt.instdir
        tmp_dirs_created = [opt.instdir]
        bldtopdir = opt.instdir.joinpath('TMPBUILD')
        if opt.tmpworkdir:
            tmp_dirs_created += [opt.tmpworkdir]
            bldtopdir = opt.tmpworkdir
        blddir = bldtopdir.joinpath('build')
        srcdir = bldtopdir.joinpath('source')
        for d in tmp_dirs_created:
            if d.exists() and not Sys.isemptydir(d):
                raise RuntimeError('ERROR: Directory must be a non-empty directory or not exist: %s'%d)
        self.__dirs_created = tmp_dirs_created
        for d in [blddir,bldtopdir,srcdir]:
            d.mkdir(parents=True,exist_ok=True)
        os.chdir(str(srcdir))
        if opt.localsrc:
            dlsrc = Sys.quote(str(opt.localsrc))
            srcislocal = True
        else:
            dlsrc = self.download_url(opt.version)
            srcislocal = not '//' in dlsrc#too simplistic url handling in general, but works for our use-cases.
        dltarget = dlsrc.split('/')[-1]#also too simplistic, but good enough here
        p='::: '
        if os.environ.get('DGDEPFIX_NOCHECK',None)!='1':
            print("::: Commencing pre-checks.")
            try:
                DG.platformutils.base_check(do_git=False,do_pipvenv=False,do_py3dev=False)
                print("Launching empty CMake project to verify basic functionality")
                DG.utils.launch_dummy_cmake('')#Empty CMake project must run.
                self.check_reqs()#Last check before download starts
            except (SystemExit,RuntimeError) as e:
                print(flush=True)
                if not isinstance(e,SystemExit) or (e.code and not isinstance(e.code,int)):
                    print(str(e))
                    print(flush=True)
                print("::: ERROR: Precheck failed. Please check output above for indications of errors (alternatively disable this check by setting DGDEPFIX_NOCHECK=1)")
                print(flush=True)
                raise SystemExit(1)
        else:
            print("::: WARNING: Skipping pre-checks since running with DGDEPFIX_NOCHECK=1")
        try:
            if srcislocal:
                print("%sFetching source via local copy of: %s"%(p,dlsrc))
                shutil.copy2(dlsrc,dltarget)
            else:
                print("%sFetching source download from: %s"%(p,dlsrc))
                Sys.download(dlsrc,dltarget)
        except Exception as e:
            print("%sERROR: Failed to get source. Error was:\n\n"%p)
            raise e

        print("\n%s: Acquired source!"%p)
        upcmd=self.unpack_command(opt.version)
        if upcmd:
            #allow * wildcard in upcmd:
            u=[]
            for i in range(len(upcmd)):
                if '*' in upcmd[i]:
                    g=glob.glob(upcmd[i])
                    assert g,"no files selected for unpacking by wildcard (%s)"%upcmd[i]
                    u+=g
                else:
                    u+=[upcmd[i]]
            upcmd = Sys.quote_cmd(u)
            print("%sUnpacking source via: %s"%(p,upcmd))
            ec=Sys.system(upcmd)
            if ec:
                print("%sERROR: Failed to unpack source."%p)
                sys.exit(ec)
        src_work_subdir = self.src_work_subdir(opt.version)
        if '*' in src_work_subdir:
            l=list(_ for _ in AbsPath('.').glob(src_work_subdir) if _.is_dir())
            assert len(l)==1,"Unexpected source-file content"
            src_work_subdir=l[0].name
        srcworkdir = srcdir.joinpath(src_work_subdir)
        if opt.dbginfo:
            new_srcworkdir=opt.instdir.joinpath('source')
            shutil.move(srcworkdir,new_srcworkdir)
            srcworkdir=new_srcworkdir
        cfgcmd = self.configure_command(opt.instdir,srcworkdir,blddir,opt.version,cfgargs)
        if cfgcmd[0]=='cmake':
            cfgcmd.insert(1,'-DPython3_FIND_VIRTUALENV=ONLY')#Never find python3 installation outside our virtualenv
        cfgcmd = Sys.quote_cmd(cfgcmd)
        os.chdir(str(srcworkdir if self.in_source_buildandconfig() else blddir))
        print("%sConfiguring via: %s"%(p,cfgcmd))
        ec=Sys.system(cfgcmd)
        if ec:
            print("%sERROR: Failure during configuration stage."%p)
            sys.exit(ec)
        if not opt.nprocs:
            opt.nprocs = DG.cpudetect.auto_njobs()
        print("%sWill use %i processes for compilation and installation."%(p,opt.nprocs))
        bldcmd = self.build_command(opt.nprocs)
        bldcmd = Sys.quote_cmd(bldcmd)
        print("%sBuilding via: %s"%(p,bldcmd))
        ec=Sys.system(bldcmd)
        if ec:
            print("%sERROR: Failure during build stage."%p)
            sys.exit(ec)
        instcmd = self.install_command(opt.nprocs)
        instcmd = Sys.quote_cmd(instcmd)
        print("%sInstalling via: %s"%(p,instcmd))
        ec=Sys.system(instcmd)
        if ec:
            print("%sERROR: Failure during installation stage."%p)
            sys.exit(ec)
        sc,usc = self._content_setup_files(opt.instdir)
        if sc is not None:
            assert usc is not None
            print("%sCreating setup.sh and unsetup.sh for usage of installation"%p)
            try:
                opt.instdir.joinpath('setup.sh').write_text(sc)
                opt.instdir.joinpath('unsetup.sh').write_text(usc)
            except Exception as e:
                print("%sERROR: Failure while trying to create setup.sh and unsetup.sh files"%p)
                print("       ",str(e))
                sys.exit(1)
        lds=[opt.instdir.joinpath(l) for l in self.libdirs_in_installation()]
        lds_to_fix = [l for l in lds if l.is_dir()]
        if lds_to_fix and sys.platform=='darwin':
            print("%sOSX detected - proceeding to fix install_names in installed libraries"%p)
            for ld in lds_to_fix:
                DG.fixosxlibinstallnames.set_install_names_to_abs_path(ld,p)
        print(p)
        instdir_quoted = Sys.quote(str(opt.instdir))
        dgdepdir = self._get_active_dgdepdir()
        print("%s%s successfully installed in:"%(p,self.name()))
        print(p)
        print("%s   %s"%(p,instdir_quoted))
        print(p)
        if dgdepdir and opt.instdir.parent.parent == dgdepdir:
            print("%sIt will be enabled whenever you (re)activate the installation at %s with:"%(p,dgdepdir))
            print(p)
            print("%s    . %s/setup.sh"%(p,dgdepdir))
            print(p)
        else:
            print("%sTo use it you can type the command (or put it in $HOME/.bashrc or $HOME/.dgcode_prebootstrap.sh):"%p)
            print(p)
            print("%s   . %s/setup.sh"%(p,instdir_quoted))
            print(p)
            print("%sTo undo the above, type instead:"%p)
            print(p)
            print("%s   . %s/unsetup.sh"%(p,instdir_quoted))
            print(p)
        print("%sIf in the future wish to remove the installation of %s, simply remove the"%(p,self.name()))
        print("%sentire directory with this command (beware, always double-check rm -rf commands!!):"%p)
        print(p)
        print("%s   rm -rf %s"%(p,instdir_quoted))
        print(p)
        if not opt.nocleanup:
            Sys.rm_rf(str(bldtopdir))
        else:
            print("%sAs requested, the build and source files are left in this directory, which"%p)
            print("%sis safe to remove later:"%p)
            print(p)
            print("%s   %s"%(p,Sys.quote(str(bldtopdir))))

#FIXME: allow --force?
#FIXME: E.g. Geant4 configuration could be nice and let us know if xercesc is missing or not (unless --nogdml).
