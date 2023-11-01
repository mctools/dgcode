#configuration file (warning: not everything can be changed here without updating other modules as well)

import os
import pathlib
import sysconfig

def AbsPath( p ):
    return pathlib.Path(p).expanduser().resolve().absolute()

__cache_pyextsuf = sysconfig.get_config_var('EXT_SUFFIX') #value like '.cpython-311-x86_64-linux-gnu.so' (.so even on osx)

lang_extensions = {
    'cxx' : ( ['hh', 'icc'], ['cc'] ),
    'c' : ( ['h'], ['c'] ),
    'fortran' : ( [], ['f'] ),
}

package_cfg_file='pkg.info'

autodeps = set(['Core'])
projectname='SBLD'
projectname_lc = projectname.lower()
runnable_prefix = 'sb_'
print_prefix_name = 'simplebuild'
print_prefix = f'{print_prefix_name}: '
quiet = False#might be overridden by a cal to make_quiet
_printfct = [print]
def print( *args, **kwargs ):
    _printfct[0]( print_prefix, *args, **kwargs )
def print_no_prefix( *args, **kwargs ):
    _printfct[0]( *args, **kwargs )
def make_quiet():
    global quiet
    if not quiet:
        quiet = True
        _printfct[0] = lambda *args, **kwargs : None

def runnable_name(pkg,base_name):
    #create global runnable name for a runnable in a package:
    return ('%s%s_%s'%(runnable_prefix,pkg.name,base_name)).lower()

def runnable_is_test(runnable_name):
    return runnable_name.split('_',2)[2].startswith('test')

def libldflag(pkg): return '-lPKG__%s'%pkg.name
def namefct_lib(pkg,subdir,platform_pattern): return platform_pattern%('PKG__'+pkg.name)
def namefct_pycpp(pkg,subdir,platform_pattern): return subdir[6:] + __cache_pyextsuf

def namefct_app(pkg,subdir,platform_pattern): return runnable_name(pkg,subdir[4:])

def checkfct_pycpp(pkg,subdir,name):
    n=subdir[6:]
    if n=='__init__':
        return 'The name pycpp___init__/ is forbidden. Instead put compiled init code in a subdir called pycpp__init/'
    cf=os.path.join(pkg.dirname,'python/%s.py'%n)
    if os.path.exists(cf):
        return 'Python module %s.%s is provided in both pure (python/%s.py) and compiled (pycpp_%s/) forms'%(pkg.name,n,n,n)

def descrfct_lib(pkg,subdir,name):
    from . import col
    return col.bldcol('shlib'),'shared library for package %s'%pkg.name

def descrfct_pycpp(pkg,subdir,name):
    from . import col
    return col.bldcol('pymod'),'python module %s.%s'%(pkg.name,subdir[6:])

def descrfct_app(pkg,subdir,name):
    from . import col
    return col.bldcol('app'),'application %s'%name

def uninstall_package(pkgname):
    #completely remove all traces of a pkg from the install area:
    #a few sanity checks since we are about to use rm -rf:
    #assert d and not ' ' in d

    instdir = install_dir()
    if not ( instdir / '.sbinstalldir' ).exists():
        return

    #FIXME: <pn>_foo_bar_blah might be script Blah from package Foo_Bar or script
    #Bar_Blah from package Foo. We should check that the symlinks goes to the
    #correct package! (or better yet, simplebuild should produce pickle file in
    #install or bld with all package dependencies and provided scripts, apps,
    #etc.). But a quick fix for the scripts (not for the apps) would be for the
    #framework to remove all symlinks to the package in question from the
    #install dir, and just let the present function deal with non-symlinks.

    parts = [ f'data/{pkgname}',
              f'lib/*PKG__{pkgname}.*',
              f'tests/testref/{runnable_prefix}{pkgname.lower()}_*.log',
              f'include/{pkgname}',
              f'python/{pkgname}',
              f'scripts/{runnable_prefix}{pkgname.lower()}_*',
              f'bin/{runnable_prefix}{pkgname.lower()}_*' ]
    import shutil
    for p in parts:
        for f in instdir.glob(p):
            if f.is_dir():
                shutil.rmtree(f,ignore_errors = True)
            else:
                f.unlink(missing_ok = True)

#Get paths to all packages (including the framework and user packages)

def projects_dir():
    from . import envcfg
# DGBUILD-EXPORT-ONLY>>    return envcfg.var.projects_dir
    proj_dir_env = envcfg.var.projects_dir # DGBUILD-NO-EXPORT
    if not proj_dir_env: # DGBUILD-NO-EXPORT
        from . import error # DGBUILD-NO-EXPORT
        error.error('The DGCODE_PROJECTS_DIR environment variable must be set.') # DGBUILD-NO-EXPORT
    proj_dir = AbsPath(proj_dir_env) # DGBUILD-NO-EXPORT
    if not proj_dir.is_absolute(): # DGBUILD-NO-EXPORT
        from . import error # DGBUILD-NO-EXPORT
        error.error('The DGCODE_PROJECTS_DIR environment variable must hold an absolute path.') # DGBUILD-NO-EXPORT
    return proj_dir # DGBUILD-NO-EXPORT

def extra_pkg_path():
    from . import envcfg
# DGBUILD-EXPORT-ONLY>>    return envcfg.var.extra_pkg_path_list
    dirs = [] # DGBUILD-NO-EXPORT
    extra_pkg_path_env = envcfg.var.extra_pkg_path # DGBUILD-NO-EXPORT
    if extra_pkg_path_env: # DGBUILD-NO-EXPORT
        dirs.extend([AbsPath(p.strip()) for p in extra_pkg_path_env.split(':') if p.strip()]) # DGBUILD-NO-EXPORT
    return dirs # DGBUILD-NO-EXPORT

#NEVERUSE # DGBUILD-EXPORT-ONLY>>def framework_dir():
#NEVERUSE # DGBUILD-EXPORT-ONLY>>    return (AbsPath(__file__).parent / 'data' / 'pkgs' / 'Framework')
def framework_dir(system_dir): # DGBUILD-NO-EXPORT
    return (AbsPath(system_dir) / '../packages/Framework').resolve() # DGBUILD-NO-EXPORT

# DGBUILD-EXPORT-ONLY>>def pkg_search_path():
# DGBUILD-EXPORT-ONLY>>    #candidates = [framework_dir(), projects_dir()]
# DGBUILD-EXPORT-ONLY>>    candidates = [projects_dir()]
# DGBUILD-EXPORT-ONLY>>    candidates.extend(extra_pkg_path())
# DGBUILD-EXPORT-ONLY>>    dirs = []
# DGBUILD-EXPORT-ONLY>>    for d in candidates:
# DGBUILD-EXPORT-ONLY>>        if not d in dirs:
# DGBUILD-EXPORT-ONLY>>            dirs.append( d )
# DGBUILD-EXPORT-ONLY>>    return dirs
def pkg_search_path(system_dir): # DGBUILD-NO-EXPORT
    dirs = [framework_dir(system_dir), projects_dir()] # DGBUILD-NO-EXPORT
    dirs.extend(extra_pkg_path()) # DGBUILD-NO-EXPORT
    return dirs # DGBUILD-NO-EXPORT

# DGBUILD-EXPORT-ONLY>>def build_dir():
# DGBUILD-EXPORT-ONLY>>    from . import envcfg
# DGBUILD-EXPORT-ONLY>>    return envcfg.var.build_dir_resolved
# DGBUILD-EXPORT-ONLY>>def install_dir():
# DGBUILD-EXPORT-ONLY>>    from . import envcfg
# DGBUILD-EXPORT-ONLY>>    return envcfg.var.install_dir_resolved

def _determine_resolved_dir( env_var_name, attribute_name ):  # DGBUILD-NO-EXPORT
    from . import envcfg
    assert hasattr(envcfg.var,attribute_name)  # DGBUILD-NO-EXPORT
    the_dir = getattr(envcfg.var,attribute_name)  # DGBUILD-NO-EXPORT
    if not the_dir:  # DGBUILD-NO-EXPORT
      from . import error  # DGBUILD-NO-EXPORT
      error.error(f'The {env_var_name} environment variable is not set.'  # DGBUILD-NO-EXPORT
                  ' Please source the bootstrap.sh file in the Projects directory once again!')  # DGBUILD-NO-EXPORT
    the_dir_real = AbsPath(the_dir)  # DGBUILD-NO-EXPORT
    if not the_dir_real.is_absolute():  # DGBUILD-NO-EXPORT
      from . import error  # DGBUILD-NO-EXPORT
      error.error(f'The {env_var_name} environment variable must hold an absolute path.')  # DGBUILD-NO-EXPORT
    return the_dir_real  # DGBUILD-NO-EXPORT

_cache_builddir = [None]  # DGBUILD-NO-EXPORT
def build_dir():  # DGBUILD-NO-EXPORT
    if _cache_builddir[0] is None:  # DGBUILD-NO-EXPORT
        _cache_builddir[0] = _determine_resolved_dir('DGCODE_BUILD_DIR_RESOLVED','build_dir_resolved')  # DGBUILD-NO-EXPORT
    return _cache_builddir[0]  # DGBUILD-NO-EXPORT

_cache_installdir = [None]  # DGBUILD-NO-EXPORT
def install_dir():  # DGBUILD-NO-EXPORT
    if _cache_installdir[0] is None:  # DGBUILD-NO-EXPORT
        _cache_installdir[0] = _determine_resolved_dir('DGCODE_INSTALL_DIR_RESOLVED','install_dir_resolved')  # DGBUILD-NO-EXPORT
    return _cache_installdir[0]  # DGBUILD-NO-EXPORT

def test_dir():
    return build_dir() / 'testresults/'

def safe_remove_install_and_build_dir():
    import shutil
    for fingerprintfile in [ build_dir() / '.sbbuilddir', install_dir() / '.sbinstalldir' ]:
        if fingerprintfile.exists():
            shutil.rmtree( fingerprintfile.parent, ignore_errors=True )

def target_factories_for_patterns():
    from . import tfact_symlink as tfs
    from . import tfact_headerdeps as tfh
    from . import tfact_binary as tfb
    l=[]
    l += [('data',   tfs.create_tfactory_symlink('','data/%s',chmodx=False))]
    l += [('scripts',tfs.create_tfactory_symlink('','scripts',chmodx=True,
                                                 renamefct=runnable_name))]#todo: disallow periods? enforce lowercase?
    l += [('python', tfs.create_tfactory_symlink('.+\.py','python/%s',chmodx=False))]
    l += [('pycpp_.+', tfb.create_tfactory_binary(shlib=True,
                                                  instsubdir='python/%s',
                                                  allowed_langs=['cxx'],
                                                  namefct=namefct_pycpp,
                                                  descrfct=descrfct_pycpp,
                                                  checkfct=checkfct_pycpp,
                                                  flagfct=lambda pkg,subdir:['-DPYMODNAME=%s'%subdir[6:]]))]
    l += [('app_.+', tfb.create_tfactory_binary(instsubdir='bin',namefct=namefct_app,
                                                  descrfct=descrfct_app))]
    l += [('libsrc', tfb.create_tfactory_binary(pkglib=True,namefct=namefct_lib,descrfct=descrfct_lib))]
    l += [('libinc',  tfh.tfactory_headerdeps)]#just for header dependencies


    return l

def ignore_file(f):
    f = f.name if hasattr(f,'name') else f
    return f[0]=='.' or '~' in f or '#' in f or f.endswith('.orig') or f.endswith('.bak') or f=='__pycache__'

def target_factories():
    #non-pattern factories:
    from . import tfact_prepinc
    from . import tfact_pyinit
    from . import tfact_reflogs
    from . import tfact_libavail
    l = []
    l += [tfact_prepinc.tfactory_prepinc]
    l += [tfact_pyinit.tfactory_pyinit]
    l += [tfact_reflogs.tfactory_reflogs]
    l += [tfact_libavail.tfactory_libavail]
    return l

def deinstall_parts(instdir,pkgname,current_parts,disappeared_parts):
    from . import dirs
    i=instdir
    if not ( i / '.sbinstalldir' ).exists():
        return
    unused=set()
    pydone=False
    pkgcache=dirs.pkg_cache_dir(pkgname)
    import shutil
    def rm_tree(p):
        shutil.rmtree( p, ignore_errors=True)
    def rm_file( p ):
        p.unlink(missing_ok = True)
    def rm_pattern(thedir,pattern):
        for f in thedir.glob(pattern):
            rm_file(f)

    for d in disappeared_parts:
        if d=='libinc':
            rm_tree( i / 'include' / pkgname)
        elif d=='libsrc':
            rm_pattern(i/'lib','*PKG__%s.*'%pkgname)
        elif d.startswith('app_'):
            rm_tree( i / 'bin' / '%s%s_%s'%(runnable_prefix,pkgname.lower(),d[4:].lower()) )
        elif d=='symlink__scripts':
            rm_pattern( i/'scripts','%s%s_*'%(runnable_prefix,pkgname.lower()))#FIXME: clashes (see fixme above)
            (pkgcache/'symlinks'/'scripts.pkl').touch()
            rm_file(pkgcache/'symlinks'/'scripts.pkl.old')
        elif d=='symlink__data':
            rm_tree( i / 'data' / pkgname )
            ( pkgcache / 'symlinks' / 'data.pkl' ).touch()
            rm_file( pkgcache / 'symlinks' / 'data.pkl.old' )
        #don't do this for testref_links, since all packages always have this target:
        #elif d=='testref_links':
        #    NB: Syntax not updated for pathlib and fcts above!:
        #    utils.rm_f(os.path.join(i,'tests/testref/%s%s_*.log'%(runnable_prefix,pkgname.lower())))
        #    utils.touch(os.path.join(pkgcache,'testref/testref.pkl'))
        #    utils.rm_f(os.path.join(pkgcache,'testref/testref.pkl.old'))
        elif d.startswith('autopyinit'):
            if not any(e.startswith('autopyinit') for e in current_parts):
                #we must remove the auto generated __init__.py as it is in the way
                rm_file( i / 'python' / pkgname / '__init__.py' )
                ( pkgcache / 'symlinks' / 'python.pkl' ).touch()
                rm_file( pkgcache / 'symlinks' / 'python.pkl.old' )
        elif d.startswith('pycpp_') or d=='symlink__python':
            if d.startswith('pycpp'):
                rm_file( i / 'python' / pkgname / '%s.so'%d[6:] )
            if not pydone:
                pydone=True
                if not any((e.startswith('pycpp') or e=='symlink__python') for e in current_parts):
                    rm_tree( i / 'python' / pkgname )
                ( pkgcache / 'symlinks' / 'python.pkl' ).touch()
                rm_file( pkgcache / 'symlinks' / 'python.pkl.old' )
                ( pkgcache / 'pyinit' / 'pyinit.pkl' ).touch()
                #rm_file( pkgcache / 'pyinit' / 'pyinit.pkl.old' )
        else:
            unused.add(d)
    assert not unused,str(unused)
