
from dgdepfixer.std import *
from dgdepfixer import allmods as DG

class OSGInstaller(DG.installutils.stdinstaller):
    def name(self): return 'OpenSceneGraph'
    def default_version(self): return '3.6.5'#NB: Update check_reqs() method below when updating this!
    def download_url(self,version):
        if version=='3.6.4':
            #Need patch which adds C++17 fixes (from https://github.com/ess-dg/OpenSceneGraph-dgpatches):
            return 'http://tkittel.web.cern.ch/tkittel/depfix/OpenSceneGraph-3.6.4_dgpatch1.zip'
        if version=='3.6.5':
            #Need patch which adds C++17 fixes (from https://github.com/ess-dg/OpenSceneGraph-dgpatches),
            #and which also fixes OSX Cocoa backend:
            return 'http://tkittel.web.cern.ch/tkittel/depfix/OpenSceneGraph-3.6.5_dgpatch1.zip'
        return 'https://github.com/openscenegraph/OpenSceneGraph/archive/OpenSceneGraph-%s.zip'%version
    def unpack_command(self,version):
        assert version
        return ['python3','-mzipfile','-e','OpenSceneGraph-%s*.zip'%version,'.']
    def src_work_subdir(self,version):
        return 'OpenSceneGraph*-%s'%version
    def in_source_buildandconfig(self): return False
    def dbg_flags(self): return ['-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_DEBUG_POSTFIX=']
    def configure_command(self,instdir,srcdir,blddir,version,extra_options):
        flags=['-DCMAKE_INSTALL_PREFIX=%s'%DG.sysutils.quote(instdir),
               '-Wno-dev',
               '-DCMAKE_CXX_STANDARD=17',
               '-DCMAKE_BUILD_TYPE=Release',
               '-DBUILD_OSG_EXAMPLES=NO',#this drags in dependencies
               '-DLIB_POSTFIX=']

        #Avoid libjpeg dependency due to incompatibility between recent libjpeg
        #and osg (related to DGSW-385). Update: disable gif/png/tiff support as
        #well, better reduce dependencies...:
        for optdep in ['JPEG','GIFLIB','PNG','TIFF']:
            if not any('CMAKE_DISABLE_FIND_PACKAGE_%s'%optdep in eo for eo in extra_options):
                flags += ['-DCMAKE_DISABLE_FIND_PACKAGE_%s=ON'%optdep]

        if sys.platform=='darwin':
            flags+=['-DCMAKE_INSTALL_NAME_DIR=%s/lib'%DG.sysutils.quote(instdir)]
        flags+=extra_options
        cmd = ['cmake']+self._prune_duplicate_flags(flags)+[str(srcdir)]
        #OSG use register keyword in two files, although forbidden in C++17. Our
        #OpenSceneGraph-3.6.*_dgpatch1.tar.gz patched version removes them, but
        #in case this script is used for other versions, we disable the
        #associated warning (which of course does not always help):
        cxxflags = ['-Wno-register']
        if sys.platform.lower()=='darwin':
            #OSX 10.14 and later started deprecation warnings in OpenGL:
            cxxflags += ['-DGL_SILENCE_DEPRECATION=1']
        if cxxflags:
            oldflags = os.environ.get('CXXFLAGS','')
            if oldflags:
                cxxflags += [oldflags]
            cmd = ['env','CXXFLAGS=%s'%(' '.join(cxxflags))] + cmd
        return cmd
    def validate_version(self,version):
        parts=version.split('.')
        if len(parts)!=3: return False
        for p in parts[0:2]:
            if not p.isdigit():
                return False
        return True

    def allow_local_source(self): return True

    def local_source_to_version(self,srcfile_basename):
        b,e='OpenSceneGraph-','.zip'
        if srcfile_basename.startswith('OpenSceneGraph-OpenSceneGraph-'):
            b='OpenSceneGraph-OpenSceneGraph-'#work with githubs oddly named files
        if not srcfile_basename.startswith(b): return None
        if not srcfile_basename.endswith(e): return None
        s=(srcfile_basename[len(b):])[:-len(e)]
        if '_dgpatch' in s:
            s = s.split('_dgpatch',1)[0]
        return s

    def setup_file_contents(self,instdir):
        return ['export OSGDIR=%s'%DG.sysutils.quote(instdir),
                'export LD_LIBRARY_PATH=$OSGDIR/lib64:$OSGDIR/lib:$LD_LIBRARY_PATH',
                'export DYLD_LIBRARY_PATH=$OSGDIR/lib64:$OSGDIR/lib:$DYLD_LIBRARY_PATH',
                'export PATH=$OSGDIR/bin:$PATH']

    def unsetup_file_contents(self,instdir):
        return ['if [ "x$OSGDIR" == "x%s" ]; then'%DG.sysutils.quote(instdir),
                '    prunepath PATH "$OSGDIR"',
                '    prunepath LD_LIBRARY_PATH "$OSGDIR"',
                '    prunepath DYLD_LIBRARY_PATH "$OSGDIR"',
                '    unset OSGDIR',
                '    unset OSG_DIR',
                '    unset OSG_ROOT',
                'fi']
    def prefix_env_var(self): return 'OSGDIR'
    def libdirs_in_installation(self): return []
    def check_reqs(self):
        l=['OpenGL']
        if sys.platform.lower()!='darwin':
            l+=['X11']#optional on darwin

        for n in l:
            print("Launching CMake to extract information about %s availability"%n)
            DG.utils.launch_dummy_cmake('find_package(%s REQUIRED)'%n)

def main():
    OSGInstaller().go()
if __name__=='__main__':
    main()


