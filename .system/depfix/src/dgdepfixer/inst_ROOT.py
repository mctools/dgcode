
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

class RootInstaller(DG.installutils.stdinstaller):
    _extraopt_nogui, _extraopt_nogdml = None,None#For pylint

    def name(self): return 'ROOT'
    def default_version(self): return '6.24.02'
    def download_url(self,version): return 'https://root.cern.ch/download/root_v%s.source.tar.gz'%version
    def unpack_command(self,version):
        assert version
        return ['python3','-mtarfile','-e','root_v%s.source.tar.gz'%version]
    def src_work_subdir(self,version): return 'root*-%s/'%version
    def in_source_buildandconfig(self): return False
    def dbg_flags(self): return ['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_DEBUG_POSTFIX=']
    def configure_command(self,instdir,srcdir,blddir,version,extra_options):
        flags=['-DCMAKE_INSTALL_PREFIX=%s'%DG.sysutils.quote(instdir),
               '-Wno-dev','-DCMAKE_CXX_STANDARD=17',
               '-DCMAKE_BUILD_TYPE=Release']
        if sys.platform.lower()=='darwin':
            ld=os.path.join(instdir,'lib')#fixme: pathlib/quote
            flags+=['-DCMAKE_INSTALL_LIBDIR=%s'%ld,'-DCMAKE_INSTALL_NAME_DIR=%s'%ld]

        is_minimal = any(opt.startswith('gminimal=') for opt in extra_options)
        flags += ['-Dgdml=%s'%('OFF' if (self._extraopt_nogdml or is_minimal) else 'ON')]

        flags += ['-Dvdt=OFF','-Ddavix=OFF']

        if self._extraopt_nogui:
            flags += ['-Dx11=OFF','-Dopengl=OFF']

        #We don't really support building without python3, but do have hidden (untested) support for minimal
        has_python = not is_minimal
        flags+=['-Dpyroot=%s'%('ON' if has_python else 'OFF')]

        if has_python:
            py3executable,py3lib,py3incdir = DG.utils.extract_py3info_via_cmake()
            flags += ['-DPYTHON_EXECUTABLE=%s'%shlex.quote(py3executable),
                      '-DPYTHON_LIBRARY=%s'%shlex.quote(py3lib),
                      '-DPYTHON_INCLUDE_DIR=%s'%shlex.quote(py3incdir)]
        flags += extra_options
        return ['cmake']+self._prune_duplicate_flags(flags)+[str(srcdir)]
    def validate_version(self,version):
        parts=version.split('.')
        if len(parts)!=3: return False
        for p in parts:
            if not p.isdigit():
                return False
        if int(parts[0])<=5:
            raise RuntimeError('Do not use this script to install ROOT versions earlier than 6.x.y.')
        return True
    def allow_local_source(self): return True
    def local_source_to_version(self,srcfile_basename):
        #extract version as root_v<VERSION>.source.tar.gz'
        b,e='root_v','.source.tar.gz'
        if not srcfile_basename.startswith(b): return None
        if not srcfile_basename.endswith(e): return None
        return (srcfile_basename[len(b):])[:-len(e)]
    def setup_file_contents(self,instdir):
        return ['export ROOTSYS=%s'%DG.sysutils.quote(instdir),
                'export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH',
                'export DYLD_LIBRARY_PATH=$ROOTSYS/lib:$DYLD_LIBRARY_PATH',
                'export PATH=$ROOTSYS/bin:$PATH',
                ###'export CMAKE_PREFIX_PATH=$ROOTSYS:{CMAKE_PREFIX_PATH}',#NB: Untested!
                'export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH']
    def unsetup_file_contents(self,instdir):
        return ['if [ "x$ROOTSYS" == "x%s" ]; then'%DG.sysutils.quote(instdir),
                '    prunepath PATH "$ROOTSYS"',
                '    prunepath LD_LIBRARY_PATH "$ROOTSYS"',
                '    prunepath DYLD_LIBRARY_PATH "$ROOTSYS"',
                '    prunepath PYTHONPATH "$ROOTSYS"',
                '    unset ROOTSYS',
                'fi']
    def prefix_env_var(self): return 'ROOTSYS'
    def libdirs_in_installation(self):
        #needed since root doesn't seem to respect the CMAKE_INSTALL_NAME_DIR
        #flag we set above. We leave both this (and 'lib64') for robustness, in
        #case of changes in future releases of root:
        return ['lib','lib64']

    def add_parser_options(self,parser,isfullhelp,hidefullfct):
        parser.add_argument("--nogui",default=False,action='store_true',dest="extraopt_nogui",
                            help='Set to disable plugins needing X11, OpenGl and Qt (which must otherwise be available on your system).')
        parser.add_argument("--nogdml",default=False,action='store_true',dest="extraopt_nogdml",
                            help='Set to build ROOT without GDML support (avoiding need for Xerces-C on your system).')
        return
    def get_parser_options(self,opt):
        setattr(self,'_extraopt_nogui',opt.extraopt_nogui)
        setattr(self,'_extraopt_nogdml',opt.extraopt_nogdml)
    def get_autoname_extras_descr(self):
        return ''.join('[_%s]'%e for e in ('nogui','nogdml'))
    def get_autoname_extras(self):
        s=''
        for e in ('nogui','nogdml'):
            if getattr(self,'_extraopt_%s'%e):
                s+='_%s'%e
        return s
    def check_reqs(self):
        #Check py3dev:
        print("Double-checking python3 installation")
        DG.platformutils.base_check(do_git=False,do_pipvenv=False,do_py3dev=True)
        if not self._extraopt_nogdml:
            print("Launching CMake to extract information about XercesC availability (since --nogdml was not supplied)")
            DG.platformutils.check_and_advice_xercesc()

        if not self._extraopt_nogui and sys.platform.lower()!='darwin':
            for n in ['X11','X11_Xpm','X11_Xft','OpenGL']:
                print("Launching CMake to extract information about %s availability (since --nogui was not supplied and this is not OSX)"%n)
                if n.startswith('X11_'):
                    DG.platformutils.check_and_advice_x11subpkg(n[4:])
                else:
                    DG.utils.launch_dummy_cmake('find_package(%s REQUIRED)'%n)


def main():
    RootInstaller().go()
if __name__=='__main__':
    main()
