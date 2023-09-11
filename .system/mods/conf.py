#configuration file (warning: not everything can be changed here without updating other modules as well)

import os
import pathlib
from . import envcfg
def AbsPath( p ):
    return pathlib.Path(p).expanduser().resolve().absolute()

lang_extensions = {
    'cxx' : ( ['hh', 'icc'], ['cc'] ),
    'c' : ( ['h'], ['c'] ),
    'fortran' : ( [], ['f'] ),
}

package_cfg_file='pkg.info'

autodeps = set(['Core','DGBoost'])# DGBUILD-NO-EXPORT
# DGBUILD-EXPORT-ONLY>>autodeps = set(['Core'])
projectname='ESS'

def runnable_name(pkg,base_name):
    #create global runnable name for a runnable in a package:
    return ('ess_%s_%s'%(pkg.name,base_name)).lower()

def runnable_is_test(runnable_name):
    return runnable_name.split('_',2)[2].startswith('test')

def libldflag(pkg): return '-lPKG__%s'%pkg.name
def namefct_lib(pkg,subdir,platform_pattern): return platform_pattern%('PKG__'+pkg.name)
def namefct_pycpp(pkg,subdir,platform_pattern): return subdir[6:]+'.so'#python mods must always be .so
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

def uninstall_package(d,pn):
    #completely remove all traces of a pkg from the install area:
    #a few sanity checks since we are about to use rm -rf:
    from . import utils
    assert d and not ' ' in d

    check_install_dir_indicator(d)
    #FIXME: ess_foo_bar_blah might be script Blah from package Foo_Bar or script
    #Bar_Blah from package Foo. We should check that the symlinks goes to the
    #correct package! (or better yet, dgbuild should produce pickle file in
    #install or bld with all package dependencies and provided scripts, apps,
    #etc.). But a quick fix for the scripts (not for the apps) would be for the
    #framework to remove all symlinks to the package in question from the
    #install dir, and just let the present function deal with non-symlinks.
    utils.system('rm -rf %s/data/%s %s/lib/*PKG__%s.*  %s/tests/testref/ess_%s_*.log %s/include/%s %s/python/%s %s/scripts/ess_%s_* %s/bin/ess_%s_*'%(d,pn,
                                                                                                                                                      d,pn,
                                                                                                                                                      d,pn.lower(),
                                                                                                                                                      d,pn,
                                                                                                                                                      d,pn,
                                                                                                                                                      d,pn.lower(),
                                                                                                                                                      d,pn.lower()))

#Get paths to all packages (including the framework and user packages)

def projects_dir():
  proj_dir_env = envcfg.var.projects_dir
  if not proj_dir_env:
      from . import error
      error.error('The DGCODE_PROJECTS_DIR environment variable must be set.')
  proj_dir = AbsPath(proj_dir_env)
  if not proj_dir.is_absolute():
      from . import error
      error.error('The DGCODE_PROJECTS_DIR environment variable must hold an absolute path.')
  return proj_dir

def extra_pkg_path():
  dirs = []
  extra_pkg_path_env = envcfg.var.extra_pkg_path
  if extra_pkg_path_env:
    dirs.extend([AbsPath(p.strip()) for p in extra_pkg_path_env.split(':') if p.strip()])
  return dirs

# DGBUILD-EXPORT-ONLY>>def framework_dir():
# DGBUILD-EXPORT-ONLY>>    return (AbsPath(__file__).parent / 'data' / 'pkgs' / 'Framework')
def framework_dir(system_dir): # DGBUILD-NO-EXPORT
    return (AbsPath(system_dir) / '../packages/Framework').resolve() # DGBUILD-NO-EXPORT

# DGBUILD-EXPORT-ONLY>>def pkg_search_path():
# DGBUILD-EXPORT-ONLY>>    dirs = [framework_dir(), projects_dir()]
# DGBUILD-EXPORT-ONLY>>    dirs.extend(extra_pkg_path())
# DGBUILD-EXPORT-ONLY>>    return dirs
def pkg_search_path(system_dir): # DGBUILD-NO-EXPORT
    dirs = [framework_dir(system_dir), projects_dir()] # DGBUILD-NO-EXPORT
    dirs.extend(extra_pkg_path()) # DGBUILD-NO-EXPORT
    return dirs # DGBUILD-NO-EXPORT

def _determine_resolved_dir( env_var_name, attribute_name ):
    assert hasattr(envcfg.var,attribute_name)
    the_dir = getattr(envcfg.var,attribute_name)
    if not the_dir:
      from . import error
      error.error(f'The {env_var_name} environment variable is not set.'
                  ' Please source the bootstrap.sh file in the Projects directory once again!')
    the_dir_real = AbsPath(the_dir)
    if not the_dir_real.is_absolute():
      from . import error
      error.error(f'The {env_var_name} environment variable must hold an absolute path.')
    return the_dir_real

_cache_builddir = [None]
def build_dir():
    if _cache_builddir[0] is None:
        _cache_builddir[0] = _determine_resolved_dir('DGCODE_BUILD_DIR_RESOLVED','build_dir_resolved')
    return _cache_builddir[0]

_cache_installdir = [None]
def install_dir():
    if _cache_installdir[0] is None:
        _cache_installdir[0] = _determine_resolved_dir('DGCODE_INSTALL_DIR_RESOLVED','install_dir_resolved')
    return _cache_installdir[0]

def test_dir():
    return AbsPath(build_dir()) / 'testresults/'

# directory indicators - empty files to indicate the build/install directory, to be checked before using rm -rf on it
def build_dir_indicator(bld_dir):
    return AbsPath(bld_dir) / '.dgbuilddir'

def install_dir_indicator(inst_dir):
    return AbsPath(inst_dir) / '.dginstalldir'

def check_build_dir_indicator(bld_dir):
  if AbsPath(bld_dir).exists() and not AbsPath(build_dir_indicator(bld_dir)).exists():
      from .import error
      error.error(f'Missing build directory indicator in {bld_dir} suggests a possible'
                  ' problem with the DGCODE_BUILD_DIR environment variable. Make sure'
                  ' you really want to delete the folder, and do it by hand!'%bld_dir)
  return True

def check_install_dir_indicator(inst_dir):
  if AbsPath(inst_dir).exists() and not AbsPath(install_dir_indicator(inst_dir)).exists():
      from .import error
      error.error(f'Missing install directory indicator in {inst_dir} suggests a possible'
                  ' problem with the DGCODE_INSTALL_DIR environment variable. Make sure'
                  ' you really want to delete the folder, and do it by hand!'%inst_dir)
  return True

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

def _del_pattern(thedir,pattern):
    import glob
    for f in glob.glob(os.path.join(thedir,pattern)):
        os.remove(f)

def deinstall_parts(instdir,pkgname,current_parts,disappeared_parts):
    from . import utils
    from . import dirs
    d=disappeared_parts
    i=instdir
    unused=set()
    pydone=False
    pkgcache=dirs.pkg_cache_dir(pkgname)
    for d in disappeared_parts:
        if d=='libinc':
            utils.rm_rf(os.path.join(i,'include',pkgname))
        elif d=='libsrc':
            _del_pattern(os.path.join(i,'lib'),'*PKG__%s.*'%pkgname)
        elif d.startswith('app_'):
            utils.rm_f(os.path.join(i,'bin/ess_%s_%s'%(pkgname.lower(),d[4:].lower())))
        elif d=='symlink__scripts':
            _del_pattern(os.path.join(i,'scripts'),'ess_%s_*'%pkgname.lower())#FIXME: clashes (see fixme above)
            utils.touch(os.path.join(pkgcache,'symlinks/scripts.pkl'))
            utils.rm_f(os.path.join(pkgcache,'symlinks/scripts.pkl.old'))
        elif d=='symlink__data':
            utils.rm_rf(os.path.join(i,'data',pkgname))
            utils.touch(os.path.join(pkgcache,'symlinks/data.pkl'))
            utils.rm_f(os.path.join(pkgcache,'symlinks/data.pkl.old'))
        #don't do this for testref_links, since all packages always have this target:
        #elif d=='testref_links':
        #    utils.rm_f(os.path.join(i,'tests/testref/ess_%s_*.log'%(pkgname.lower())))
        #    utils.touch(os.path.join(pkgcache,'testref/testref.pkl'))
        #    utils.rm_f(os.path.join(pkgcache,'testref/testref.pkl.old'))
        elif d.startswith('autopyinit'):
            if not any(e.startswith('autopyinit') for e in current_parts):
                #we must remove the auto generated __init__.py as it is in the way
                autoi=os.path.join(i,'python',pkgname,'__init__.py')
                if os.path.exists(autoi):
                    os.remove(autoi)
                utils.touch(os.path.join(pkgcache,'symlinks/python.pkl'))
                utils.rm_f(os.path.join(pkgcache,'symlinks/python.pkl.old'))
        elif d.startswith('pycpp_') or d=='symlink__python':
            if d.startswith('pycpp'):
                utils.rm_f(os.path.join(i,'python/%s/%s.so'%(pkgname,d[6:])))
            if not pydone:
                pydone=True
                if not any((e.startswith('pycpp') or e=='symlink__python') for e in current_parts):
                    utils.rm_rf(os.path.join(i,'python',pkgname))
                utils.touch(os.path.join(pkgcache,'symlinks/python.pkl'))
                utils.rm_f(os.path.join(pkgcache,'symlinks/python.pkl.old'))
                utils.touch(os.path.join(pkgcache,'pyinit/pyinit.pkl'))
                #utils.rm_f(os.path.join(pkgcache,'pyinit/pyinit.pkl.old'))
        else:
            unused.add(d)
    assert not unused,str(unused)
