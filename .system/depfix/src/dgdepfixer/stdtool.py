#!/usr/bin/env python3

from dgdepfixer.std import *
from dgdepfixer import allmods as DG

from dgdepfixer.sysutils import AbsPath,ResolvedAbsPath,SafeResolvedAbsPath,system_throw

defaultdir = "~/dgdepfixer_install/"#recognised by dgcode's bootstrap.sh
defaultdir_resolved = SafeResolvedAbsPath(defaultdir)

__version_fingerprint = json.dumps(('dgdepfixer-%s'%DG.__version__,),sort_keys=True)#embed in installations to detect (in)compatibility with current script version

#From Core.System:
def rm_rf(path):
    #First attempt with os.remove, and only in case it was a directory go for
    #shutil.rmtree (since shutil.rmtree prints error messages if fed symlinks to
    #directories):
    try:
        os.remove(path)
    except OSError as e:
        if e.errno == errno.ENOENT:#no such file or directory
            return#not an error, rm -rf should exit silently here
        elif e.errno != errno.EISDIR and e.errno!=errno.EPERM:
            raise
    #is apparently a directory
    try:
        shutil.rmtree(path)
    except OSError as e:
        if e.errno == errno.ENOENT:
            return#Sudden disappearance is still ok.
        raise

def has_ok_cmake():
    try:
        v = system_throw('cmake --version',True).split()[2]
    except RuntimeError:
        v = None
    return not DG.platformutils.version_issue('CMake',v)

def parse_cmdline():

    parser = argparse.ArgumentParser(description="""Script for debugging an environment and setting up some of the dependencies required by dgcode,
                                                    ensuring a modern CMake, Git a standard set of python3 modules available and possibly
                                                    other optional software as well. Launch with no arguments and follow the instructions to get started.""")


    #Custom solution for hiding some options unless requested (Note that --fu*
    #is enough to select --fullhelp, but --f is not due to the presence of
    #--force):
    full_help = any(a.startswith('--fu') for a in sys.argv[1:])
    hide = (lambda msg : msg) if full_help else (lambda _ : argparse.SUPPRESS)
    extra_args = ['--help'] if full_help else []
    parser.add_argument('--fullhelp', action='store_true',help="""Show help for expert-only options.""")

    parser.add_argument('-v','--version', action='store_true',help="""Show dgdepfixer version.""")

    parser.add_argument("-d", "--dir",metavar='DIR',
                        type=AbsPath, dest="instdir", default=AbsPath(defaultdir),
                        help=hide("""Install in DIR (default %s) when running with --create. It is usually
                                     a good idea to leave this at the default value"""%defaultdir))
    parser.add_argument('-f','--force', action='store_true',help=hide("""Remove any existing installation dir (see --dir
                                                                         argument) before commencing."""))
    parser.add_argument('-c','--create', action='store_true',help="Create installation of customisations needed to bring system up to minimal"
                        +" standards%s. NOTE: Normal users should first run without using this flag in order to debug their system."%(' (this will happen in DIR, cf. the --dir option)' if full_help else ''))
    parser.add_argument('-b','--bleedingedge', action='store_true',help=hide("""Attempt to install latest bleeding edge
                                                                                versions of all python modules"""))
    parser.add_argument('--cmake', choices=['always','never','ifneeded'],
                        default='ifneeded',help=hide("Whether to install cmake when using --create (default: %(default)s)."))
    parser.add_argument('--diagnose', action='store_true',
                        help=hide("""Use to diagnose the current environment. Returns 0 if all is ok, 99 in case of problems that
                                     can be fixed by this script and other non-zero codes in case of issues beyond salvation. All
                                     other arguments will be ignored when this flag is specified."""))
    parser.add_argument('--summarise', action='store_true',
                        help=hide('Use when running from existing dgdepfixer installation (i.e. DGDEPDIR set)'
                                  ' to present a brief summary - in particular concerning optional SW.'))
    parser.add_argument('--checkcompat', metavar='DIR',
                        type=AbsPath, dest="checkcompatdir", default=None,
                        help=hide("Check compatibility of installation in DIR and exit with a code of 0(compatible) or 1 (incompatible)"))
    parser.add_argument('--extras',metavar='LIST',type=str,dest='extralist',default='',
                        help=hide("Option which can be supplied with --create only. It must be given a comma-separated list of names of"
                                  " optional SW to install. Example, if the user supplies --extras=Geant4,OpenSceneGraph then the"
                                  " dgdepfixer --instsw=Geant4 and dgdepfixer --instsw=OpenSceneGraph commands will be run automatically"
                                  " at the appropriate time."))
    parser.add_argument('--instsw', choices=DG.utils.allowed_swnames(),
                        default=None,help=hide("For installing additional optional SW once everything else is working"
                                                     " (use with --help or --fullhelp for additional SW-specific options)."))

    args=parser.parse_args(sys.argv[1:]+extra_args)
    nmodes = sum((1 if a else 0) for a in (args.checkcompatdir,args.diagnose,args.create,args.summarise,args.version))
    if nmodes>1:
        parser.error('Supply at most one of the --diagnose, --create, --checkcompat, --version, and --summarise flags')

    if args.instsw is not None:
        parser.error('ERROR: --instsw must be the first argument')

    if args.extralist.strip() and not args.create:
        parser.error('Option --extras can only be used with --create')

    if not nmodes:
        class Opts:
            mode = 'intro'
        return Opts()

    if args.version:
        class Opts:
            mode = 'version'
        return Opts()

    if args.diagnose:
        class Opts:
            mode = 'diagnose'
        return Opts()

    if args.summarise:
        class Opts:
            mode = 'summarise'
        return Opts()

    if args.checkcompatdir:
        if not args.checkcompatdir.exists() or not args.checkcompatdir.is_dir():
            parser.error('Please supply existing directory to --checkcompat option')
        class Opts:
            mode = 'checkcompat'
            checkcompatdir = args.checkcompatdir
        return Opts()


    _pymods_for_pip = ['%s==%s'%(k[6:],v['recommended']) for k,v in sorted(DG.swversions.items()) if k.startswith('pymod:')]
    if args.bleedingedge:
        _pymods_for_pip=[m.split('==')[0] for m in _pymods_for_pip]
    _extrasw = list(e.strip().lower() for e in args.extralist.strip().split(','))
    class Opts:
        mode = 'create'
        instdir = args.instdir
        force = bool(args.force)
        pymods_for_pip = _pymods_for_pip
        cmake = args.cmake
        extrasw = list(sorted(set(e for e in _extrasw if e)))
    return Opts()

def _cmake_detect_platform():
    m=platform.machine()
    if m in ['i486','i586','i686']:
        m='i386'
    p=sys.platform
    if p.lower().startswith('linux'):
        p='Linux'
    elif p.lower().startswith('darwin'):
        p='Darwin'
    return p,m

def _cmake_tarbase_name(version):
    p,m=_cmake_detect_platform()
    return 'cmake-%s-%s-%s'%(version,p,m)

def cmakeurl(version=None):
    if version is None:
        version = DG.swversions['CMake']['recommended']
    #vmajor,vminor,_=version.split('.')
    tb = _cmake_tarbase_name(version)
    if tb in ('cmake-3.14.4-Darwin-x86_64','cmake-3.14.4-Linux-x86_64'):
        #Previous versions, kept here for reference (TODO: remove them?):
        return 'https://tkittel.web.cern.ch/tkittel/depfix/%s.tar.gz'%tb

    if tb in ('cmake-3.15.3-Darwin-x86_64','cmake-3.15.3-Linux-x86_64'):
        #Cache most commonly needed files at CERN, to avoid any potential github
        #throttling issues when users need to retry a few times for whatever
        #reasons (to be fair, this issue was only observed when the releases
        #were hosted at cmake.org):
        return 'https://tkittel.web.cern.ch/tkittel/depfix/%s.tar.gz'%tb

    return 'https://github.com/Kitware/CMake/releases/download/v%s/%s.tar.gz'%(version,tb)
    #return 'https://cmake.org/files/v%s.%s/%s.tar.gz'%(vmajor,vminor,tb)

def detect_issues():
    try:
        sysinfo=DG.platformutils.probe_system_versions()
    except BaseException as e:
        print("\n\n\nOups - Unexpected error!!\n\n\n")
        raise e
    issues=[]
    for depkey in sorted(DG.swversions.keys()):
        pymod = depkey[6:] if depkey.startswith('pymod:') else None
        dep_pretty = 'Python3 module %s'%pymod if pymod else depkey
        issue_detected = DG.platformutils.version_issue(depkey,sysinfo[depkey])
        if issue_detected:
            issues += ['%s %s'%(dep_pretty,issue_detected)]
    return issues

def diagnose_installation_and_exit(quiet=False):
    try:
        issues = detect_issues()
    except SystemExit as e:
        #Guard against accidental usage of exit code 99:
        if e.code == 99:
            e.code = 98
            raise e
    if issues and not quiet:
        for i in issues:
            errprint('Problem detected: %s'%i)
    sys.exit(99 if issues else 0)

def active_dgdepdir_path():
    active_dgdepdir=os.environ.get('DGDEPDIR',None)
    return SafeResolvedAbsPath(active_dgdepdir) if active_dgdepdir else None

def locate_existing_installs(complete_search=False):
    """Search for existing installation directories created by dgdepdir. In addition
       to checking the default installation locations and the DGDEPDIR
       environment variable, it will also look recursively under HOME and
       CWD. If complete_search=False the recursive search will be limited to a
       few levels deep. This will avoid hangups.

    """

    fingerprint='.dgcode_dependencies_installation'
    incomplete_search_lvls = 5
    yielded = set()
    #always check the active location if any:
    active_dgdepdir=active_dgdepdir_path()
    if active_dgdepdir and active_dgdepdir.exists():
        if active_dgdepdir.joinpath(fingerprint).exists():
            yielded.add(active_dgdepdir)
            yield active_dgdepdir

    #and always check default location:
    f=defaultdir_resolved
    if f.joinpath(fingerprint).exists():
        if not f in yielded:
            yielded.add(f)
            yield f



    #Now search home dir for other installations:
    home=SafeResolvedAbsPath(pathlib.Path.home())

    #Do not auto-descend into certain dirs (unrelated or triggers permissions requests on OSX 10.15):
    forbidden = [ 'Desktop', 'Documents', 'Downloads', 'Library', 'Movies',
                  'Music', 'Pictures', 'Videos', 'Private', 'Templates', 'Trash',
                  'bin', 'lib', 'snap', 'dg_dgcode*', 'mnt*' ]

    #Add all allowed dirs from home:
    search_top = []
    for _ in sorted(list(set(SafeResolvedAbsPath(__) for __ in home.glob('*') if __.is_dir() ))):
        if not any(fnmatch.fnmatch(_.name,fb) for fb in forbidden) and not _.name.startswith('.'):
            search_top += [_]

    if complete_search:
        #slow but inclusive: recursive glob
        for st in search_top:
            for f in st.glob('**/%s'%fingerprint):
                p=ResolvedAbsPath(f).parent
                if not p in yielded:
                    yielded.add(p)
                    yield p
    else:
        #a recursive glob finds all, but might take too long - we limit the
        #search to a few levels deep.
        for st in search_top:
            for pattern in ['*/'*i for i in range(1,incomplete_search_lvls)]:
                for f in st.glob('%s%s'%(pattern,fingerprint)):
                    p=ResolvedAbsPath(f).parent
                    if not p in yielded:
                        yielded.add(p)
                        yield p

def py3fingerprint():
    #Invalidate installation if the python installation changes too much. We
    #used to use sys.version rather than sys.version, but it has too many
    #details, thus invalidating the installation needlessly in case of minor OS
    #updates.
    #
    #NOTE: For now we manually keep this in sync with the setup.sh code below.
    return str([sys.version_info[0:2],os.__file__])

def inspect_existing_install(p):
    """Return dictionary with status information of existing installation"""
    #FIXME: Script could have --search / --searchall flags for finding other installations (possibly in specified paths, like /opt) and reporting on them.
    d={}

    d['defaultdir'] = defaultdir_resolved==SafeResolvedAbsPath(p)
    d['incomplete'] = not all(p.joinpath(f).exists() for f in ('.dgcode_dependencies_installation',
                                                               '.dgcode_dependencies_installation_done',
                                                               '.dgdepfixerfingerprint',
                                                               '.py3fingerprint',
                                                               'setup.sh',
                                                               'unsetup.sh'))
    active_dgdepdir=active_dgdepdir_path()
    d['active'] = active_dgdepdir and active_dgdepdir==ResolvedAbsPath(p)

    timestampfile = p.joinpath('.dgcode_dependencies_installation')
    d['timestamp'] = datetime.datetime.fromtimestamp(timestampfile.stat().st_mtime) if timestampfile.exists() else None


    if not d['incomplete']:
        d['incompatible_py3'] = bool(p.joinpath('.py3fingerprint').read_text() != py3fingerprint())
        d['incompatible_dgdepfixer'] = bool(p.joinpath('.dgdepfixerfingerprint').read_text() !=__version_fingerprint)
    else:
        d['incompatible_py3'] = False
        d['incompatible_dgdepfixer'] = False
    d['compatible'] = not ( d['incompatible_py3'] or d['incompatible_dgdepfixer'] or d['incomplete'] )

    assert set(d.keys())=={'active','timestamp','incomplete','incompatible_py3','incompatible_dgdepfixer','compatible','defaultdir'}
    return d

def do_diagnose_mode():
    """Mode mainly intended for system verification in dgcode's bootstrap.sh"""
    DG.platformutils.base_check()
    diagnose_installation_and_exit()

def find_optional_installations(extras_dir):
    #Will even find those that are incomplete
    d={}
    for n in DG.utils.allowed_swnames():
        variations=['%s-*'%n,'%s_*'%n,'%s-*'%n.lower(),'%s_*'%n.lower()]
        d[n] = sum((list(extras_dir.glob(v)) for v in variations),[])
    return d

def do_summarise_mode():
    """Mode mainly intended for system summary when sourcing a <DGDEPDIR>/setup.sh file"""
    quiet = ('DGDEPDIR_QUIET' in os.environ)
    dgdepdir = active_dgdepdir_path()
    if not dgdepdir:
        raise SystemExit('--summarise should only be used with an active DGDEPDIR installation.')

    installs_all = find_optional_installations(dgdepdir.joinpath('extras'))
    for n in DG.utils.allowed_swnames():
        installs = installs_all[n]
        for i in installs[:]:
            if not i.joinpath('setup.sh').exists():
                print("WARNING: ------------------------------------------------------------------------------------------------------")
                print("WARNING: Incomplete installation found at %s"%i)
                print('WARNING: You should probably remove it by running the following command (and always be careful with "rm -rf"!):')
                print("WARNING:    rm -rf %s"%shlex.quote(str(i)))
                print("WARNING: ------------------------------------------------------------------------------------------------------")
                installs.remove(i)
        if not installs:
            if not quiet:
                print('Optional SW not installed: %s (can be installed with command: "dgdepfixer --instsw=%s")'%(n,n))
            continue
        if len(installs)>1:
            print("WARNING: Multiple versions of %s installed into the same dgdepfixer installation. That should not happen!:"%n)
        else:
            if not quiet:
                print("Optional SW installed: %s"%installs[0].name)

def do_checkcompat(p):
    d = inspect_existing_install(p)
    return d['compatible']

def do_intro_mode():
    """Primary mode intended for normal users - will help the user set up their system (including finally directing them towards using the --create mode to set up a custom cmake + venv etc.)"""
    #TODO: Insert sleep statements between stages, to give users a chance to read!!!

    progname = os.path.basename(sys.argv[0])

    print('\n::: Welcome to %s - a tool for helping you fix your system so it can run ESS Detector Group code.\n'%progname)
    print('::: Please send comments, questions, ideas for improvements to %s !\n'%DG.__email__)

    print('::: Performing basic system verification...\n')
    bugmsg='This is likely to be a bug, please report to %s (include a copy of all output above in your report).'%DG.__email__
    try:
        DG.platformutils.base_check()
    except KeyboardInterrupt:
        print("\n::: Basic system verification aborted by user interrupt! You should rerun %s to continue system validation."%progname)
        return 127
    except SystemExit as e:
        if not e.code==1:
            m=str(e)
            if m.lower().startswith('error:'):
                m=m[6:].strip()
            if m:
                print("ERROR: %s"%m)
        print('\n::: Basic system verification failed! Try to follow any instructions above to fix your system (if any), then rerun %s.'%progname)
        return 1
    except BaseException as e:
        traceback.print_exc()
        print()
        print('\n::: Basic system verification failed with unknown exception! %s'%bugmsg)
        return 1
    print('::: Completed basic system verification!\n')

    print('::: Performing extended system verification...\n')

    try:
        issues = detect_issues()
    except BaseException as e:
        print("\n\n\nOups - Unexpected error!!\n\n\n")
        traceback.print_exc()
        print('\n::: Extended system verification failed with unknown exception! %s'%bugmsg)
        return 1
    if issues:
        for i in issues:
            print('Problem detected: %s'%i)
        print()

    print('::: Completed extended system verification%s!'%(' - %i unresolved issues found'%len(issues) if issues else ' - No issues found'))

    print('\n::: Attempting to locate existing installations created by %s scripts...\n'%progname)

    existing_installations = []
    for p in locate_existing_installs(complete_search=False):
        info = inspect_existing_install(p)
        existing_installations += [(info,p)]
        if not info['compatible']:
            if info['incomplete']:
                reason = 'incomplete installation'
            elif info['incompatible_py3']:
                reason = 'created with incompatible python interpreter'
            else:
                assert info['incompatible_dgdepfixer']
                reason = 'created with incompatible version of dgdepfixer script'
        if info['active']:
            summary = 'ACTIVE but INCOMPATIBLE (reason: %s)'%reason if not info['compatible'] else 'ACTIVE and COMPATIBLE'
        else:
            summary = 'INCOMPATIBLE (reason: %s)'%reason if not info['compatible'] else 'COMPATIBLE'
        print ("Found %s installation from %s at: %s"%(summary,info['timestamp'].strftime('%Y-%m-%dT%H:%M'),p))
    print('\n::: Found %i such installations!\n'%len(existing_installations))

    active = [(d,p) for d,p in existing_installations if d['active']]
    assert len(active)<=1
    active_path = active[0][1] if active else None
    active_at_defaultdir = active and active[0][0]['defaultdir']


    if active and not active[0][0]['compatible']:
        info,p = active[0]
        print("::: ERROR: The installation at %s is currently active despite being incompatible."%p)
        print(":::")
        f=p.joinpath('unsetup.sh')
        if f.exists():
            print(":::          You should deactivate it by typing the command:\n:::")
            print(":::          . %s\n:::"%f)
            print(":::          Afterwards, try to run %s again.\n"%progname)
        else:
            print(":::          It seems to be so messed up that it can not simply be deactivated!!!\n:::")
            print(":::          Advice: Remove the directory at %s, close and restart your terminal, then run %s again.\n"%(p,progname))
        return 1

    #ok, any active installation is compatible. Time to check if all is good and exit:
    if not issues:
        #Fixme: wrap printouts
        if not active:
            print("::: Your system seems to be satisfying all requirements even without an active installation")
            print("::: created by %s --create. This is a bit unusual, but probably good news!\n"%progname)
        if len(existing_installations)>len(active):
            print("::: NOTE: As listed above, you seems to have unused (inactive and/or incompatible) installations.")
            print(":::       Unless you have them around for some particular reason, it might be safe to remove them")
            print(":::       and recover some disk space.\n")
        if active and not active_at_defaultdir:
            print("::: WARNING: The currently active installation at %s will NOT"%active_path)
            print(":::          be picked up by default by dgcode's bootstrap.sh since it was created in an unusual location!")
            print(":::          Either recreate the installation at %s (i.e. launch %s --create"%(defaultdir,progname))
            print(":::          WITHOUT using the --dir argument) or add the following line to a file like ~/.bashrc or")
            print(":::          ~/.dgcode_prebootstrap.sh :")
            print(":::")
            print(":::          . %s/setup.sh"%active_path)
            print(":::")
            print(":::          (if you already did this, you can of course ignore this warning)\n")
        print("::: No lingering system issues found! Congratulations!\n")
        if os.environ.get('DGCODE_DIR',None) or pathlib.Path('.system/mods/frontend.py').exists():
            print("::: To continue, you can now source the bootstrap.sh file in a local clone of dg_dgcode and start working.")
        else:
            print("::: To continue, source the bootstrap.sh file in a local clone of dg_dgcode.")
            print("::: If you haven't checked out such a clone already, you can do so with the following command:\n:::")
            print(":::   git clone https://MYUSERNAME@bitbucket.org/europeanspallationsource/dg_dgcode\n:::")
            print("::: (remember to change MYUSERNAME to your actual bitbucket username)")
        return 0

    assert issues
    #There are issues, we should advice the user to --create an installation which can fix this.

    if active:
        assert active[0][0]['compatible']
        print("::: ERROR: Extended system verification detected lingering issues, but you already seem to ")
        print(":::        be using a compatible installation created with %s --create."%progname)
        print(":::        You can anyway try to see if recreating the installation and activating it will")
        print(":::        solve your issues (which it might if you updated something on your system). Thus, run:")
        print(":::")
        print(":::        . %s/unsetup.sh"%active_path)
        print(":::        %s --create --force%s"%(progname,'' if active_at_defaultdir else '--dir=%s'%shlex.quote(active[0][1])))
        print(":::        . %s/setup.sh"%active_path)
        print(":::")
        print(":::        And then run %s again. If that doesn't solve the issue, please contact %s for help (include a copy of all output above in your report)"%(progname,DG.__email__))
        return 1

    #Could activate?
    compatible = [(d,p) for d,p in existing_installations if d['compatible']]
    if compatible:
        if len(compatible)==1:
            print("::: You seem to have a compatible but unused installation. You can try to activate it with this command:\n:::")
            print(":::    . %s/setup.sh"%compatible[0][1])
        else:
            print("::: You seem to have compatible but unused installations. You can try to activate one of them with one of these commands:\n:::")
            for d,p in compatible:
                print(":::    . %s/setup.sh%s"%(p,'     # <-- the recommended one based one location' if d['defaultdir'] else ''))
        print(":::\n::: Afterwards you can try to run %s again and see if it helped."%progname)
        return 1

    somethingintheway = defaultdir_resolved.exists() and not DG.sysutils.isemptydir(defaultdir_resolved)
    print("::: To fix the issues above you should now run the command%s:"%(' (--force will remove existing files at %s so be careful!!)'%defaultdir if somethingintheway else ''))
    print(":::\n:::      %s --create%s"%(progname,' --force' if somethingintheway else ''))
    print(":::\n::: Afterwards you can source the new setup.sh as you will be instructed and then try to run %s again and see if it helped."%progname)
    return 1

def do_create_mode(tempdir,instdir,force,cmake,pymods_for_pip):

    progname = os.path.basename(sys.argv[0])

    ################################################################
    #### Determine path to the current dgdepfixer "binary" file ####
    #### (compiling a fresh one if running from sources).       ####
    ################################################################

    thisfile = AbsPath(__file__)#might be /path/to/<dgdepfixerzipfile>/dgdepfixer/stdtool.py so can't resolve yet
    if thisfile.parts[-5:]== ('.system', 'depfix', 'src', 'dgdepfixer','stdtool.py'):
        #Special case: running directly from src in dgcode. Compile a "binary" and refer to it.
        system_throw('%s ALWAYS'%shlex.quote(str(thisfile.parent.joinpath('../../compile.py'))))
        dgdepfixerfile = thisfile.parent.joinpath('../../bld/bin/dgdepfixer')
    else:
        #already running from a compiled dgdepfixer file, discard trailing virtual entries (apparently system or python version dependent):
        while not thisfile.exists() and len(thisfile.parts)>2:
            thisfile=thisfile.parent
        dgdepfixerfile = thisfile
        if dgdepfixerfile.is_dir() or not dgdepfixerfile.exists():
            raise SystemExit('Error detected: The dgdepfixer script could not determine its own identity...')

    ################################################################
    #### Safety: don't create new installations from dgdepfixer ####
    #### residing in a static installation. Not only does it    ####
    #### lead to funny problems when the active installation    ####
    #### is deactivated or deleted, but it also means the       ####
    #### user is likely to create new installations using       ####
    #### obsolete dgdepfixer versions.                          ####
    ################################################################

    if dgdepfixerfile.parent.name == 'dgdepfixer_bin' and dgdepfixerfile.parent.parent.joinpath('.dgdepfixerfingerprint').exists():
        errprint("ERROR: You should NOT create new dgdepfixer installations by running dgdepfixer out")
        errprint("       of an existing dgdepfixer installation. Instead, download an updated version")
        errprint("       and run it by invoking the following three commands:")
        errprint("")
        errprint("curl -OSsf http://tkittel.web.cern.ch/tkittel/depfix/dgdepfixer")
        errprint("chmod +x ./dgdepfixer")
        errprint("./dgdepfixer %s"%(' '.join(shlex.quote(a) for a in sys.argv[1:])))
        errprint("")
        sys.exit(1)

    ##############################
    #### Basic platform check ####
    ##############################

    DG.platformutils.base_check()

    #########################################################
    #### Safety: don't create new installations from ########
    #### active venvs.                               ########
    #########################################################

    _ = os.environ.get('DGDEPDIR',None)
    if _:
        _ = AbsPath(_).joinpath('unsetup.sh')
        if _.exists():
            raise SystemExit('ERROR: Deactivate existing installation before proceeding by running this command:\n       . %s'%_)

    if os.environ.get('VIRTUAL_ENV',None):
        raise SystemExit('ERROR: Deactivate existing virtual environment before proceeding by running this command:\n       deactivate')


    #########################################################
    ################## Prepare directory ####################
    #########################################################

    idfile = instdir.joinpath('.dgcode_dependencies_installation')
    instdir_abs = AbsPath(instdir)
    if instdir_abs.exists() and not DG.sysutils.isemptydir(instdir_abs):
        if force:
            if not idfile.exists():
                raise SystemExit('ERROR: Existing directory "%s" does not look like it was created by this script, so can not be removed with --force. Remove it manually if you are sure.'%instdir)
            rm_rf(str(instdir_abs))
        else:
            #fixme: If this would remove any Geant4/ROOT/OSG installation we should perhaps warn about it.
            raise SystemExit('ERROR: Found existing directory "%s". Remove before proceeding\n       or rerun with the --force flag to remove it automatically.'%instdir)

    assert not instdir_abs.exists() or DG.sysutils.isemptydir(instdir_abs)
    print ("\n::: Installing into %s"%instdir)
    instdir_abs.mkdir(parents=True, exist_ok=True)
    assert DG.sysutils.isemptydir(instdir_abs)
    idfile.touch()
    print()

    #########################################################
    ######## Add dgdepfixer itself #########
    #########################################################

    setup_sh=[]
    extra_env_vars = {}

    print ("::: Adding copy of currently running dgdepfixer executable")
    bytes_starts_with = lambda b,s: b[:len(s)]==s
    if not bytes_starts_with(dgdepfixerfile.read_bytes(),b'#!/usr/bin/env python3'):
        raise SystemExit('ERROR: Problems finding myself!?!?')
    target = instdir_abs.joinpath('dgdepfixer_bin/dgdepfixer')
    target.parent.mkdir()
    shutil.copy2(str(dgdepfixerfile),str(target))
    DG.sysutils.chmod_x(target)
    setup_sh += ['export PATH="${DGDEPDIR}/dgdepfixer_bin:${PATH}"']
    print()

    #########################################################
    ############## Add fingerprints and README ##############
    #########################################################

    print ("::: Adding README")

    with instdir_abs.joinpath('.dgdepfixerfingerprint').open('wt') as fp:
        fp.write(__version_fingerprint)

    instdir_abs.joinpath('.py3fingerprint').write_text(py3fingerprint())

    with instdir_abs.joinpath('README').open('wt') as fp:
        fp.write("""
This is a custom installation created with %s --create.

Sourcing setup.sh will likely give you updated versions of tools
like CMake and certain python3 modules, and might also provide other
SW like Geant4, ROOT or OpenSceneGraph, or tools with which to compile
them.

If you think you don't need this installation any longer, it is safe to
remove it.

For questions, comments or ideas, contact %s

"""%(progname,DG.__email__))
    print()

    #########################################################
    ######################### CMake #########################
    #########################################################

    assert cmake in ('ifneeded','always','never')
    do_cmake = (not has_ok_cmake()) if (cmake=='ifneeded') else (cmake=='always')
    if do_cmake:
        print ("::: Installing CMake")
        cmakesrctar=tempdir.joinpath('cmakesrc.tar.gz')
        DG.sysutils.download(cmakeurl(),cmakesrctar,prefix='::: ')
        print ('')
        cmakeunpackdir = tempdir.joinpath('cmakeunpack')
        cmakeunpackdir.mkdir()
        with tarfile.open(str(cmakesrctar)) as tf:
            tf.extractall(path=str(cmakeunpackdir))
            #Now, search inside the extracted until we find bin/cmake (on Linux
            #they are inside a subdir named like the tarball basename, and on
            #OSX one must proceed further into a CMake.app/Contents subdir):
            cmakebin = list(cmakeunpackdir.glob('**/bin/cmake'))
            assert len(cmakebin)==1
            cmakesrcdir=cmakebin[0].parent.parent
            cmake_root = instdir_abs.joinpath('cmake')
            for sd in ['bin','doc','share','man']:
                shutil.move(str(cmakesrcdir.joinpath(sd)),str(cmake_root.joinpath(sd)))

        cmakereldir = cmake_root.relative_to(instdir_abs)
        setup_sh += ['export PATH="${DGDEPDIR}/%s/bin:${PATH}"'%str(cmakereldir)]
        extra_env_vars['CMAKE_ROOT']='"${DGDEPDIR}/"%s'%shlex.quote(str(cmakereldir))

        print ("::: CMake downloaded and unpacked.")
    else:
        print ("::: Skipping installation of CMake")
    print()

    #########################################################
    ############## Python3 venv + pip packages ##############
    #########################################################

    #setup python3 venv:
    print("::: Installing custom virtual environment for Python3")
    os.environ['VIRTUAL_ENV_DISABLE_PROMPT']='1'
    venvdir=instdir_abs.joinpath('venv')
    import venv
    with_pip = DG.platformutils.can_do_venv_with_pip()
    venv.create(str(venvdir), system_site_packages=False, symlinks=True, with_pip=with_pip)
    setup_sh+=['VIRTUAL_ENV_DISABLE_PROMPT=1 . "${DGDEPDIR}/venv/bin/activate"']
    setup_sh+=['#make sure venv deactivation and reactivation wont throw away other changes to PATH by unsetting _OLD_VIRTUAL_PATH:']
    setup_sh+=['unset _OLD_VIRTUAL_PATH || true']
    setup_sh+=['if [ -w ${DGDEPDIR}/venv/pyvenv.cfg ]; then']
    setup_sh+=['    [[ -n "$DGDEPDIR_QUIET" ]] || echo "Custom Python3 virtual environment activated. You can install more PyPI packages into it with: python3 -mpip install NAMEOFPKG"']
    setup_sh+=['else']
    setup_sh+=['    [[ -n "$DGDEPDIR_QUIET" ]] || echo "Custom (locked) Python3 virtual environment activated."']
    setup_sh+=['fi']
    venvact='VIRTUAL_ENV_DISABLE_PROMPT=1 . %s'%shlex.quote(str(venvdir.joinpath('bin/activate')))

    if not with_pip:
        d=venvdir.joinpath('getpip')
        d.mkdir()
        print("\n::: Installing pip into virtual environment via download of get-pip.py\n")
        DG.sysutils.download('https://bootstrap.pypa.io/get-pip.py', str(d.joinpath('get-pip.py')), prefix='::: ')
        print("\n\n::: Invoking get-pip.py\n")
        system_throw(venvact+' && cd %s && python3 get-pip.py'%d)

    print("\n::: Upgrading pip\n")
    system_throw(venvact+' && python3 -mpip install --upgrade pip')
    for m in pymods_for_pip:
        print("\n::: pip installing %s\n"%m)
        system_throw(venvact+' && python3 -mpip install %s'%shlex.quote(m))
    print()

    #Add pyqt5 to lib path (doesn't hurt if not installed):
    setup_sh+=['','#Make pyqt5 work without conflicts in case user installs it into our venv:']
    setup_sh+=['for i in "${DGDEPDIR}"/venv/lib/python*/site-packages/; do']
    setup_sh+=['    if [ -d "$i" ]; then']
    setup_sh+=['        export LD_LIBRARY_PATH="${i}/PyQt5/Qt/lib:$LD_LIBRARY_PATH"']
    setup_sh+=['        export DYLD_LIBRARY_PATH="${i}/PyQt5/Qt/lib:$DYLD_LIBRARY_PATH"']
    setup_sh+=['    fi']
    setup_sh+=['done']

    #########################################################
    ################ Python3 ptpython prompt ################
    #########################################################

    print("\n::: Creating PYTHONSTARTUP file for ptpython-enhancement\n")
    pystartupfile = instdir_abs.joinpath('venv/custom_pystartup.py')
    pystartupfile.write_text("""
import sys as _sys
if _sys.version_info[0:2] >= (3, 5):
    import sys
    del _sys
    import os
    import math
    import pathlib
    _stdmods = ['os', 'sys', 'math', 'pathlib']
    try:
        import numpy
        np = numpy
        _stdmods += ['numpy(=np)']
    except ImportError:
        pass
    print()
    print('Welcome to ptpython-enhanced Python3 (courtesy of dgdepfixer).')
    print('Preimported for convenience: %s'%(' '.join(_stdmods)))
    del _stdmods
    print('Note: unset PYTHONSTARTUP to disable these customisations.')
    print()
    print('---------------------------------------------------------------')
    print()
    try:
        from ptpython.repl import embed as _ptpython_embed
    except ImportError:
        print("ERROR: ptpython is not available: falling back to standard prompt")
        print()
    else:
        sys.exit(_ptpython_embed( history_filename = pathlib.Path.home() / ".py3hist",
                                  globals=globals(), locals=locals() ) )
else:
    del _sys
""")

    extra_env_vars['PYTHONSTARTUP']=str(pystartupfile)

    extra_env_vars.update(DG.platformutils.detect_and_set_extra_env_vars_needed())

    if extra_env_vars:
        setup_sh+=['','#Extra environment variables needed on this platform:']
        #First store old values so unsetup.sh can properly restore them:
        setup_sh += list("""
for DGDEPDIR_tmpvarname in <varnames>; do
    DGDEPDIR_tmpvarnameorig="_DGDEP_ORIG_${DGDEPDIR_tmpvarname}"
    if [ -z "${!DGDEPDIR_tmpvarnameorig+x}" ]; then
       export "_DGDEP_ORIG_${DGDEPDIR_tmpvarname}"="${!DGDEPDIR_tmpvarname-_u_n_s_e_t_}"
    fi
done
unset DGDEPDIR_tmpvarname
unset DGDEPDIR_tmpvarnameorig
""".replace('<varnames>',' '.join(sorted(extra_env_vars.keys()))).splitlines())
        #Now just set them:
        for k,v in sorted(extra_env_vars.items()):
            setup_sh+=[ 'export %s=%s'%(k,shlex.quote(v) if not '${DGDEPDIR}' in v else v)]


    #########################################################
    ##################### Add setup.sh ######################
    #########################################################

    print("::: Adding setup.sh\n")

    setupsh=instdir_abs.joinpath('setup.sh')
    with setupsh.open('wt') as sf:
        sf.write("""#!/bin/bash

if [ "${SHELL:-x}" != /bin/bash ]; then
    echo "ERROR: Your SHELL must be /bin/bash!"
    return 1
fi

if [ -f "${DGDEPDIR:-/:}/unsetup.sh" ]; then
    . ${DGDEPDIR}/unsetup.sh
    echo
fi

export DGDEPDIR_tmp="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CI_PLATFORM=
if [[ ! -z "${GITHUB_SERVER_URL}" ]] && [[ "$CI" = true ]]; then 
    CI_PLATFORM="GITHUB RUNNER"
fi

if [ "$CI_PLATFORM" != "GITHUB RUNNER" ]; then
    python3 - <<EOF
import sys,os,pathlib
fpref=pathlib.Path(os.environ['DGDEPDIR_tmp']).joinpath('.py3fingerprint').read_text()
fp=str([sys.version_info[0:2],os.__file__])
sys.exit(0 if fpref==fp else 1)
EOF
else
    true
fi

if [ $? != 0 ]; then
    echo "ERROR: This dgcode dependencies setup was created with a different installation of python3 than the one currently in the path."
    unset DGDEPDIR_tmp
    return 1
else
    export DGDEPDIR="${DGDEPDIR_tmp}"
    unset DGDEPDIR_tmp
""")
        setup_sh += list("""

#Enable optional extra builds if available:
for i in "$DGDEPDIR/extras"/*/setup.sh; do
    if [ -f "$i" ]; then
        ii=$( basename $( dirname "$i" ) )
        [[ -n "$DGDEPDIR_QUIET" ]] || echo "Enabling $ii"
        . $i
    fi
done
if [ "$CI_PLATFORM" != "GITHUB RUNNER" ]; then
dgdepfixer --summarise
fi
""".splitlines())

        for l in setup_sh:
            sf.write('    %s\n'%l if l else '\n')
        sf.write('fi\n')

    #########################################################
    ##################### Add unsetup.sh ######################
    #########################################################

    print("::: Adding unsetup.sh\n")
    unsetupsh=instdir_abs.joinpath('unsetup.sh')

    lines_for_unset = ''
    if extra_env_vars:
        lines_for_unset="""
#Restore certain variables to their original values:
for DGDEPDIR_tmpvarname in <varnames>; do
    DGDEPDIR_tmpvarnameorig="_DGDEP_ORIG_${DGDEPDIR_tmpvarname}"
    if [ ! -z "${!DGDEPDIR_tmpvarnameorig+x}" ]; then
        if [ "x${!DGDEPDIR_tmpvarnameorig}" == "x_u_n_s_e_t_" ]; then
            unset "${DGDEPDIR_tmpvarname}"
        else
            export "${DGDEPDIR_tmpvarname}"="${!DGDEPDIR_tmpvarnameorig}"
        fi
        unset "${DGDEPDIR_tmpvarnameorig}"
    fi
done
unset DGDEPDIR_tmpvarname
unset DGDEPDIR_tmpvarnameorig

""".replace('<varnames>',' '.join(sorted(extra_env_vars.keys())))

    with unsetupsh.open('wt') as sf:
        sf.write("""#!/bin/bash

if [ "${SHELL:-x}" != /bin/bash ]; then
    echo "ERROR: Your SHELL must be /bin/bash!"
    return 1
fi

export DGDEPDIR_tmptmp="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
for i in "$DGDEPDIR_tmptmp/extras"/*/unsetup.sh; do
    if [ -f "$i" ]; then
        ii=$( basename $( dirname "$i" ) )
        [[ -n "$DGDEPDIR_QUIET" ]] || echo "Disabling $ii"
        . "$i"
    fi
done
function prunepath() {
    P=$(IFS=:;for p in ${!1:-}; do [[ $p != ${2}* ]] && echo -n ":$p"; done)
    export $1=${P:1:99999}
}
if [ "x${DGDEPDIR}" == "x${DGDEPDIR_tmptmp}" ]; then
    unset DGDEPDIR
    [[ -n "$DGDEPDIR_QUIET" ]] || echo "Disabling custom Python3 virtual environment"
    deactivate 1>/dev/null 2>/dev/null
    #deactivate is a function defined by venv's bin/activate
fi

<lines_for_unset>
#Finally, prune references to this installation from various PATH variables. Note: For
#added robustness we do this *after* the call to venv "deactivate" above, since it might
#restore the PATH variable to it's previous state (for even more robustness we also
#prune the _OLD_VIRTUAL_PATH variable which the venv uses to do this):
echo "Removing all references to ${DGDEPDIR_tmptmp} from environment."
for DGDEPDIR_tmpvarname in PATH PYTHONPATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH _OLD_VIRTUAL_PATH _OLD_VIRTUAL_PYTHONPATH; do
    if [ -n ${!DGDEPDIR_tmpvarname:-} ] ; then
        prunepath ${DGDEPDIR_tmpvarname} "${DGDEPDIR_tmptmp}"
    fi
done
#The following variables might point to something inside (some might have been reset already, rechecking here for robustness):
for DGDEPDIR_tmpvarname in ROOTSYS NCRYSTALDIR VIRTUAL_ENV G4LEVELGAMMADATA G4RADIOACTIVEDATA G4PIIDATA G4SAIDXSDATA G4ABLADATA G4REALSURFACEDATA G4NEUTRONHPDATA G4NEUTRONXSDATA G4ENSDFSTATEDATA G4LEDATA; do
    if [[ "x${!DGDEPDIR_tmpvarname}" == "x${DGDEPDIR_tmptmp}/"* ]]; then
        unset ${DGDEPDIR_tmpvarname}
    fi
done
unset DGDEPDIR_tmptmpvarname
unset DGDEPDIR_tmptmp
""".replace('<lines_for_unset>',lines_for_unset))


    #########################################################
    ########## Finalise: mark success + emit msgs ###########
    #########################################################

    print("::: Finalising.\n")
    #mark installation as having completed successfully:
    instdir_abs.joinpath('.dgcode_dependencies_installation_done').touch()
    emit_final_msg(instdir)

    sys.exit(0)

def emit_final_msg(instdir,extras_installed=None):
    if extras_installed is None:
        extras_installed = []

    instdir_abs = ResolvedAbsPath(instdir)
    setupsh = instdir_abs.joinpath('setup.sh')

    extras_avail = DG.utils.allowed_swnames()
    extras_installed = [DG.utils.map_swname_casings(swn) for swn in extras_installed]
    progname = os.path.basename(sys.argv[0])

    assert setupsh.exists()
    print('\n'*4)
    print(('\n'+'v'*80)*3)
    print(('\n'+'='*80)*3)
    print('\n::: Installation finished succesfully. To use it, source the setup.sh file within, e.g.:\n:::')
    print(':::   . %s'%setupsh)
    print(':::\n::: Afterwards you will have access to commands for installation of additional optional SW:\n:::')


    for e in extras_avail:
        print(":::      %s --instsw=%s%s"%(progname,e,(' (already installed!)' if e in extras_installed else '')))

    print()

    setupsh_canonical = AbsPath(defaultdir).joinpath('setup.sh')
    if setupsh_canonical.exists() and setupsh_canonical.samefile(setupsh):
        print("::: Note: As you did not change the default location, dgcode's bootstrap.sh can pick it up automatically.\n")
    else:
        print("::: Note: As you used a custom installation location, dgcode's bootstrap.sh will NOT pick it up automatically.\n")

    print("::: Once you are done running %s and %s --instsw=XXX commands, you"%(progname,progname))
    print("::: can source the bootstrap.sh file again in a local dg_dgcode repo clone.")
    if not (os.environ.get('DGCODE_DIR',None) or pathlib.Path('.system/mods/frontend.py').exists()):
        print(":::\n::: If you haven't checked one out already, you can do so with the following command:\n")
        print(":::    git clone https://MYUSERNAME@bitbucket.org/europeanspallationsource/dg_dgcode\n")
        print("::: (remember to change MYUSERNAME to your actual bitbucket username)")

def main(tempdir):

    assert tempdir.is_dir()

    opts = parse_cmdline()

    #Go silent if requested:
    #if opts.quiet:
    #    sys.stdout.flush()
    #    sys.stderr.flush()
    #    devnull = open(os.devnull, 'w')#problem: must make sure devnull variable does not get cleaned up.
    #    os.dup2(devnull.fileno(), sys.stdout.fileno())
    #    os.dup2(devnull.fileno(), sys.stderr.fileno())

    if opts.mode=='intro':
        do_intro_mode()#FIXME: Catch any unexpected exceptions and add advice about emailing DG.__email__
    elif opts.mode=='diagnose':
        do_diagnose_mode()
    elif opts.mode=='summarise':
        do_summarise_mode()
    elif opts.mode=='version':
        print(DG.__version__)
        sys.exit(0)
    elif opts.mode=='checkcompat':
        sys.exit(0 if do_checkcompat(opts.checkcompatdir) else 1)
    else:
        assert opts.mode=='create'
        extras_asked = list(sorted(set(opts.extrasw)))
        bad_extrasw = [e for e in extras_asked if not DG.utils.swname_allowed(e)]
        if bad_extrasw:
            raise SystemExit('ERROR: Invalid SW name%s provided to --extras flag: %s'%('' if len(bad_extrasw)==1 else 's',' '.join(bad_extrasw)))
        extras_asked = [ DG.utils.map_swname_casings(sw) for sw in extras_asked ]
        try:
            do_create_mode(tempdir,opts.instdir,opts.force,opts.cmake,opts.pymods_for_pip)
        except KeyboardInterrupt:
            errprint('Command aborted by user!!! Directory at %s is likely in an inconsistent state now and should be removed.'%opts.instdir)
            sys.exit(1)
        extras_installed = []
        user_abort = False
        for sw in extras_asked:
            print(('\n'+'v'*80)*3)
            print(('\n'+'='*80)*3)
            cmd='dgdepfixer --instsw=%s'%sw
            os.environ['DGDEPDIR_QUIET']='1'# . setup.sh below should not emit summaries
            if user_abort:
                print('::: Skipping due to user abort: %s'%cmd)
                continue
            print('::: Launching: %s'%cmd)
            try:
                system_throw(". %s && %s"%(opts.instdir.joinpath('setup.sh'),cmd))
            except (RuntimeError,KeyboardInterrupt) as e:
                if isinstance(e,KeyboardInterrupt):
                    user_abort = True
                errprint('Installation of %s failed%s!'%(sw,' (Due to user abort!)' if user_abort else ''))
                #Cleanup:
                for d in find_optional_installations(opts.instdir.joinpath('extras'))[sw]:
                    print()
                    print("Removing directory: %s"%d)
                    rm_rf(str(d))
                continue
            extras_installed += [sw]
        if extras_asked:
            emit_final_msg(opts.instdir,extras_installed)
        if len(extras_asked)!=extras_installed:
            print()
            for sw in extras_asked:
                if sw not in extras_installed:
                    errprint("\nERROR NOTICE: Failed to install %s. You can try again via the dgdepfixer --instsw=%s command and investigate yourself!"%(sw,sw))
            sys.exit(1)
