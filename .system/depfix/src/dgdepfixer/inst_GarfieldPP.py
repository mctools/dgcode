
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

class GarfieldPPInstaller(DG.installutils.stdinstaller):
    def name(self): return 'GarfieldPP'
    def default_version(self):
        #NB: Version can be branch name, tag name or commit sha value.
        return '3.0'
    def download_url(self,version):
        #NB: Version can be branch name, tag name or commit sha value.
        return 'https://gitlab.cern.ch/garfield/garfieldpp/-/archive/%s/garfieldpp-%s.tar.bz2'%(version,version)
    def unpack_command(self,version):
        assert version
        return ['python3','-mtarfile','-e','garfieldpp-%s*.tar.bz2'%version]
    def src_work_subdir(self,version):
        return 'garfieldpp-*'
    def in_source_buildandconfig(self): return False
    def dbg_flags(self): return ['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_DEBUG_POSTFIX=']
    def configure_command(self,instdir,srcdir,blddir,version,extra_options):
        flags=['-DCMAKE_INSTALL_PREFIX=%s'%DG.sysutils.quote(instdir),
               '-Wno-dev',
               '-DCMAKE_CXX_STANDARD=17',
               '-DCMAKE_BUILD_TYPE=Release']
        if sys.platform=='darwin':
            flags+=['-DCMAKE_INSTALL_NAME_DIR=%s/lib'%DG.sysutils.quote(instdir)]
        flags+=extra_options
        return ['cmake']+self._prune_duplicate_flags(flags)+[str(srcdir)]
    def validate_version(self,version):
        return bool(version)#anything can be ok (branch names, tag names, commit sha)
    def allow_local_source(self): return True
    def local_source_to_version(self,srcfile_basename):
        b,e='garfieldpp-','.tar.bz2'
        if not srcfile_basename.startswith(b): return None
        if not srcfile_basename.endswith(e): return None
        return (srcfile_basename[len(b):])[:-len(e)]
    def _quote(self,s):
        from shlex import quote as _shlex_quote
        quote = lambda s : _shlex_quote(str(s) if hasattr(s,'__fspath__') else s)
        return quote(s)
    def setup_file_contents(self,instdir):
        return ['export GARFIELD_HOME=%s'%DG.sysutils.quote(instdir),
                'export HEED_DATABASE=$GARFIELD_HOME/Heed/heed++/database',
                'export LD_LIBRARY_PATH=$GARFIELD_HOME/lib:$LD_LIBRARY_PATH',
                'export DYLD_LIBRARY_PATH=$GARFIELD_HOME/lib:$DYLD_LIBRARY_PATH',
                'export CMAKE_PREFIX_PATH=$GARFIELD_HOME:$CMAKE_PREFIX_PATH']
    def unsetup_file_contents(self,instdir):
        return ['if [ "x$GARFIELD_HOME" == "x%s" ]; then'%DG.sysutils.quote(instdir),
                '    prunepath LD_LIBRARY_PATH "$GARFIELD_HOME"',
                '    prunepath DYLD_LIBRARY_PATH "$GARFIELD_HOME"',
                '    prunepath CMAKE_PREFIX_PATH "$GARFIELD_HOME"',
                '    unset GARFIELD_HOME',
                '    unset HEED_DATABASE',
                'fi']
    def prefix_env_var(self): return 'GARFIELD_HOME'
    def libdirs_in_installation(self): return ['lib']
    def check_reqs(self):
        if not shutil.which('root-config'):
            raise SystemExit('ERROR: root-config not found. ROOT must be installed (with '
                             +'"dgdepfixer --instsw=ROOT") and enabled before proceeding.')
        rootsys=DG.sysutils.ResolvedAbsPathOrNone(os.environ.get('ROOTSYS',None))
        if not rootsys or not (rootsys/'setup.sh').exists() or not 'dgdepfixer --instsw=ROOT' in (rootsys/'setup.sh').read_text():
            raise SystemExit('ERROR: root-config was found, but corresponding ROOT installation seems to not have been'
                             ' created with "dgdepfixer --instsw=ROOT". Please fix this before proceeding.')
def main():
    GarfieldPPInstaller().go()
if __name__=='__main__':
    main()


