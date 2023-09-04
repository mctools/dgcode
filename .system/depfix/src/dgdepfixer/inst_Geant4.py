
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

class Geant4Installer(DG.installutils.stdinstaller):
    _extraopt_gui, _extraopt_nogdml,_extraopt_nodata,_extraopt_mt = None,None,None,None#For pylint

    def name(self): return 'Geant4'
    def default_version(self): return '11.0.3'
    def download_url(self,version):
        if version in ['10.00.p03','10.00.p04'] and sys.platform=='darwin':
            #Need specially patched tar-ball to work around mac bug:
            return 'http://tkittel.web.cern.ch/tkittel/geant4.%s_fixbug1707.tar.gz'%version
        if version=='10.04.p03':
            #Need patch which backports C++17 fixes to 10.4.3 (from https://github.com/ess-dg/geant4-dgpatches):
            return 'http://tkittel.web.cern.ch/tkittel/depfix/geant4.%s_dgpatch1.tar.gz'%version
        if version.startswith('11.') or version.startswith('v'):
            old_p = any( e.startswith('.p') for e in version.split('.') )
            old_00 = any( ( len(e)>1 and e.startswith('0') ) for e in version.split('.') )
            if old_p or old_00:
                raise SystemExit('Error: use syntax 11.0.3 or v11.0.3 rather than 11.00.p03')
            _ = version if version.startswith('v') else 'v'+version
            return 'https://gitlab.cern.ch/geant4/geant4/-/archive/%s/geant4-%s.tar.gz'%(_,_)
        return 'http://cern.ch/geant4-data/releases/geant4.%s.tar.gz'%version

    def unpack_command(self,version):
        assert version
        return ['python3','-mtarfile','-e','geant4*%s*.tar.gz'%version]
    def src_work_subdir(self,version): return 'geant4*%s/'%version
    def in_source_buildandconfig(self): return False
    def dbg_flags(self): return ['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_DEBUG_POSTFIX=','-DCMAKE_CXX_FLAGS_DEBUG=-g','-UG4FPE_DEBUG']
    def configure_command(self,instdir,srcdir,blddir,version,extra_options):
        flags=[ '-DCMAKE_INSTALL_PREFIX=%s'%instdir,
                '-Wno-dev',
                '-DCMAKE_CXX_STANDARD=17',
                '-DCMAKE_BUILD_TYPE=Release',
                 #(see https://geant4-forum.web.cern.ch/t/how-to-install-g4-python-environment-for-mac-g4py/3538/6)
                '-DBUILD_SHARED_LIBS=ON',#seen in the geant4-feedstock conda-forge recipe so using here for safety
               ]

        if version.startswith('10.') or version.startswith('v10.'):
            g4cxxstd_val = 'c++17' if (version.startswith('10.04') or version.startswith('10.00')) else '17'
            flags += [ '-DGEANT4_BUILD_CXXSTD=%s'%g4cxxstd_val ]#=c++17 confirmed to work with g4 10.04, and =17 with 10.07.
        else:
            flags += [ '-DCMAKE_CXX_STANDARD_REQUIRED=ON' ]

        if not any('GEANT4_USE_SYSTEM_CLHEP' in eo for eo in extra_options):
            flags +=['-DGEANT4_USE_SYSTEM_CLHEP=OFF']
        if not any('GEANT4_USE_SYSTEM_EXPAT' in eo for eo in extra_options):
            flags +=['-DGEANT4_USE_SYSTEM_EXPAT=ON']
        if not self._extraopt_nogdml:
            flags +=[ '-DGEANT4_USE_GDML=ON' ]
        if not self._extraopt_nodata:
            flags +=['-DGEANT4_INSTALL_DATA=ON','-DGEANT4_INSTALL_DATA_TIMEOUT=10000']
        if self._extraopt_gui:
            flags += ['-DGEANT4_USE_OPENGL_X11=ON',
                      '-DGEANT4_USE_RAYTRACER_X11=ON',
                      '-DGEANT4_USE_QT=ON',
                      '-DGEANT4_USE_XM=ON']

        if self._extraopt_mt or version.startswith('11.') or version.startswith('v11.'):
            flags += [ '-DGEANT4_BUILD_MULTITHREADED=ON',
                       '-DGEANT4_BUILD_TLS_MODEL=global-dynamic',#Needed when launching G4 from Python
                      ]

        flags+=extra_options
        return ['cmake']+self._prune_duplicate_flags(flags)+[str(srcdir)]
    def validate_version(self,version):
        parts=version.split('.')
        if len(parts) not in (2,3): return False
        for p in parts[0:2]:
            if not p.isdigit():
                return False
        return True
    def allow_local_source(self): return True
    def local_source_to_version(self,srcfile_basename):
        b,e='geant4.','.tar.gz'
        if not srcfile_basename.startswith(b): return None
        if not srcfile_basename.endswith(e): return None
        s=(srcfile_basename[len(b):])[:-len(e)]
        if '_dgpatch' in s:
            s = s.split('_dgpatch',1)[0]
        return s
    def setup_file_contents(self,instdir):
        return ['export GEANT4DIR=%s'%DG.sysutils.quote(instdir),
                'source $GEANT4DIR/bin/geant4.sh']
    def unsetup_file_contents(self,instdir):
        return ['if [ "x$GEANT4DIR" == "x%s" ]; then'%DG.sysutils.quote(instdir),
                '    prunepath PATH "$GEANT4DIR"',
                '    prunepath LD_LIBRARY_PATH "$GEANT4DIR"',
                '    prunepath DYLD_LIBRARY_PATH "$GEANT4DIR"',
                '    prunepath PYTHONPATH "$GEANT4DIR"',
                '    unset GEANT4DIR',
                'fi']
    def prefix_env_var(self): return 'GEANT4DIR'
    def libdirs_in_installation(self): return ['lib','lib64']

    def add_parser_options(self,parser,isfullhelp,hidefullfct):
        parser.add_argument("--gui",default=False,action='store_true',dest="extraopt_gui",
                            help=hidefullfct('Set to enable plugins needing X11, OpenGl and Qt (these must be available on your system).'))
        parser.add_argument("--nogdml",default=False,action='store_true',dest="extraopt_nogdml",
                            help='Set to build Geant4 without GDML support (avoiding need for Xerces-C on your system).')
        parser.add_argument("--nodata",default=False,action='store_true',dest="extraopt_nodata",
                            help=hidefullfct('Set to disable download and installation of Geant4 process data (expert users only!).'))
        parser.add_argument("--mt",default=False,action='store_true',dest="extraopt_mt",
                            help=hidefullfct('Set to build Geant4 with with support for multi-threading. NOTE: MT builds will ALWAYS be enabled for G4 11.x.y'))
        return
    def get_parser_options(self,opt):
        setattr(self,'_extraopt_gui',opt.extraopt_gui)
        setattr(self,'_extraopt_nogdml',opt.extraopt_nogdml)
        setattr(self,'_extraopt_nodata',opt.extraopt_nodata)
        setattr(self,'_extraopt_mt',opt.extraopt_mt)
    def get_autoname_extras_descr(self):
        return ''.join('[_%s]'%e for e in ('gui','nogdml'))#Not 'nodata', since data will often be fixed up manually
    def get_autoname_extras(self):
        s=''
        for e in ('gui','nogdml','mt'):#Not 'nodata', since data will often be fixed up manually
            if getattr(self,'_extraopt_%s'%e):
                s+='_%s'%e
        return s
    def check_reqs(self):
        print("Launching CMake to extract information about EXPAT availability.")
        DG.platformutils.check_and_advice_expat()
        if not self._extraopt_nogdml:
            print("Launching CMake to extract information about XercesC availability (since --nogdml was not supplied).")
            DG.platformutils.check_and_advice_xercesc()


def main():
    Geant4Installer().go()
if __name__=='__main__':
    main()
