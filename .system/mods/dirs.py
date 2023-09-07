#global variables
import pathlib
from os import path
join=path.join
from . import conf#project specific configuration

#system dir is one up from the modules dir:
sysdir = str(pathlib.Path(__file__).resolve().absolute().parent.parent)
fmwkdir = conf.framework_dir(sysdir)
blddir = conf.build_dir()
blddir_indicator = conf.build_dir_indicator(blddir)
makefiledir = blddir / 'makefiles'

extrapkgpath = conf.extra_pkg_path()
pkgsearchpath = conf.pkg_search_path(sysdir)

installdir = conf.install_dir()
installdir_indicator = conf.install_dir_indicator(installdir)
testdir = conf.test_dir()
projdir = conf.projects_dir()
cmakedetectdir = pathlib.Path(sysdir) / 'cmakedetect' # DGBUILD-NO-EXPORT
# DGBUILD-EXPORT-ONLY>>datadir = pathlib.Path(__file__).resolve().absolute().parent / 'data'
# DGBUILD-EXPORT-ONLY>>cmakedetectdir = datadir / 'cmake'

incdirname='include'
#libdirname='lib'#fixme: unused, 'lib' is simply hardcoded in a few places.

sysinc_shippedboost = join(sysdir,'include')
envcache = blddir / 'env.cache'
varcache = blddir / 'vars.cache' #dynamic user settings (for pkg filters and cmake flags)
systimestamp_cache=blddir / 'systimestamp.cache'
setupfile_cache=blddir / 'setup.sh'
lockfile=blddir / ".lock"

def makefile_instdir(*subpaths):
    if not subpaths:
        return "${INST}"
    subpaths=join(*subpaths)
    if subpaths[0]=='/':
        subpaths=path.relpath(subpaths,installdir)
    return join("${INST}",subpaths)

def makefile_blddir(*subpaths):
    if not subpaths:
        return "${BLD}"
    subpaths=join(*subpaths)
    if subpaths[0]=='/':
        subpaths=path.relpath(subpaths,blddir)
    return join("${BLD}",subpaths)

def _pkgname(pkg):
    return pkg.name if hasattr(pkg,'name') else pkg

def pkg_cache_dir(pkg,*subpaths):
    return blddir.joinpath('pc',_pkgname(pkg),*subpaths)

def makefile_pkg_cache_dir(pkg,*subpaths):
    return join('${BLD}','pc',_pkgname(pkg),*subpaths)

#where we link (or create dynamic pkgs):
pkgdirbase = blddir / 'pkgs'
def pkg_dir(pkg,*subpaths): return join(pkgdirbase,_pkgname(pkg),*subpaths)
def makefile_pkg_dir(pkg,*subpaths): return join('${PKG}',_pkgname(pkg),*subpaths)

#sanity:
for d in [str(x) for x in [sysdir, blddir, *pkgsearchpath, installdir]]:
    assert not ' ' in d, 'Spaces not allowed in directory names. Error with "%s"'%d 

# Package directory aliases #keep them lowercase
pkgdir_aliases = {
  "framework": fmwkdir,
  "projects": projdir,
  "extra": extrapkgpath
  }

from .utils import mkdir_p,touch

def create_bld_dir():
  mkdir_p(blddir)
  touch(blddir_indicator)
  assert blddir.exists()
  assert blddir_indicator.exists()

def create_install_dir():
  mkdir_p(installdir)
  touch(installdir_indicator)
  assert installdir_indicator.exists()
