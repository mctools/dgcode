
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

class NCrystalInstaller(DG.installutils.stdinstaller):
    def name(self): return 'NCrystal'
    def default_version(self): return '2.2.1'
    def download_url(self,version):
        return 'https://github.com/mctools/ncrystal/archive/v%s.tar.gz'%version
    def unpack_command(self,version):
        assert version
        return ['python3','-mtarfile','-e','v%s*.tar.gz'%version]
    def src_work_subdir(self,version):
        return 'ncrystal-%s'%version
    def in_source_buildandconfig(self): return False
    def dbg_flags(self): return ['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_DEBUG_POSTFIX=']
    def configure_command(self,instdir,srcdir,blddir,version,extra_options):
        flags=['-DCMAKE_INSTALL_PREFIX=%s'%DG.sysutils.quote(instdir),
               '-Wno-dev',
               '-DCMAKE_CXX_STANDARD=17',
               '-DCMAKE_BUILD_TYPE=Release']
        if not any('BUILD_G4HOOKS' in eo for eo in extra_options):
            flags += ['-DBUILD_G4HOOKS=OFF']#For now we just provide bare-bones NCrystal for standalone or McStas usage, G4 users are expected to use dgcode.
        if sys.platform=='darwin':
            flags+=['-DCMAKE_INSTALL_NAME_DIR=%s/lib'%DG.sysutils.quote(instdir)]
        flags+=extra_options
        return ['cmake']+self._prune_duplicate_flags(flags)+[str(srcdir)]
    def validate_version(self,version):
        parts=version.split('.')
        if len(parts)!=3: return False
        for p in parts[0:2]:
            if not p.isdigit():
                return False
        return True
    def allow_local_source(self): return True
    def local_source_to_version(self,srcfile_basename):
        b,e='v','.tar.gz'
        if not srcfile_basename.startswith(b): return None
        if not srcfile_basename.endswith(e): return None
        return (srcfile_basename[len(b):])[:-len(e)]
    def setup_file_contents(self,instdir):
        return None#NCrystal already creates setup.sh - returning None disables setup.sh/unsetup.sh creation
    def unsetup_file_contents(self,instdir):
        return None
    def libdirs_in_installation(self): return ['lib']

def main():
    NCrystalInstaller().go()
if __name__=='__main__':
    main()


