#global variables
import pathlib
import os
from . import conf#project specific configuration

#system dir is one up from the modules dir:
sysdir = str(pathlib.Path(__file__).resolve().absolute().parent.parent)
fmwkdir = conf.framework_dir(sysdir)# DGBUILD-NO-EXPORT
#NEVERUSE # DGBUILD-EXPORT-ONLY>>fmwkdir = conf.framework_dir()
blddir = conf.build_dir()
makefiledir = blddir / 'makefiles'

extrapkgpath = conf.extra_pkg_path()
pkgsearchpath = conf.pkg_search_path(sysdir)# DGBUILD-NO-EXPORT
# DGBUILD-EXPORT-ONLY>>pkgsearchpath = conf.pkg_search_path()

installdir = conf.install_dir()
testdir = conf.test_dir()
projdir = conf.projects_dir()
cmakedetectdir = pathlib.Path(sysdir) / 'cmakedetect' # DGBUILD-NO-EXPORT
# DGBUILD-EXPORT-ONLY>>datadir = pathlib.Path(__file__).resolve().absolute().parent / 'data'
# DGBUILD-EXPORT-ONLY>>cmakedetectdir = datadir / 'cmake'

incdirname='include'
#libdirname='lib'#fixme: unused, 'lib' is simply hardcoded in a few places.

envcache = blddir / 'env.cache'
varcache = blddir / 'vars.cache' #dynamic user settings (for pkg filters and cmake flags)
systimestamp_cache=blddir / 'systimestamp.cache'
lockfile=blddir / ".lock"

def makefile_instdir(*subpaths):
    if not subpaths:
        return "${INST}"
    subpaths=os.path.join(*subpaths)
    if subpaths[0]=='/':
        subpaths=os.path.relpath(subpaths,installdir)
    return os.path.join("${INST}",subpaths)

def makefile_blddir(*subpaths):
    if not subpaths:
        return "${BLD}"
    subpaths=os.path.join(*subpaths)
    if subpaths[0]=='/':
        subpaths=os.path.relpath(subpaths,blddir)
    return os.path.join("${BLD}",subpaths)

def _pkgname(pkg):
    return pkg.name if hasattr(pkg,'name') else pkg

def pkg_cache_dir(pkg,*subpaths):
    return blddir.joinpath('pc',_pkgname(pkg),*subpaths)

def makefile_pkg_cache_dir(pkg,*subpaths):
    return os.path.join('${BLD}','pc',_pkgname(pkg),*subpaths)

#where we link (or create dynamic pkgs):
pkgdirbase = blddir / 'pkgs'
def pkg_dir(pkg,*subpaths):
    return pkgdirbase.joinpath( _pkgname(pkg),*subpaths )

def makefile_pkg_dir(pkg,*subpaths):
    return os.path.join('${PKG}',_pkgname(pkg),*subpaths)

#sanity:
for d in [str(x) for x in [blddir, *pkgsearchpath, installdir]]:
    assert ' ' not in d, 'Spaces not allowed in directory names. Illegal path is: "%s"'%d
    assert len(d)>3,f"suspiciously short path name: {d}"

# Package directory aliases #keep them lowercase
pkgdir_aliases = {
  "framework": fmwkdir,# DGBUILD-NO-EXPORT
  "projects": projdir,
  "extra": extrapkgpath
  }

def create_bld_dir():
    fingerprint = blddir  / '.dgbuilddir'
    if  blddir.exists() and not fingerprint.exists():
        from . import error
        error.error(f'Did not find expected fingerprint file at: {fingerprint}')
    blddir.mkdir(parents=True,exist_ok=True)
    fingerprint.touch()
    assert blddir.is_dir()
    assert ( blddir  / '.dgbuilddir' ).exists()

def create_install_dir():
    fingerprint = installdir  / '.dginstalldir'
    if  installdir.exists() and not fingerprint.exists():
        from . import error
        error.error(f'Did not find expected fingerprint file at: {fingerprint}')
    installdir.mkdir(parents=True,exist_ok=True)
    fingerprint.touch()
    assert installdir.is_dir()
    assert ( installdir  / '.dginstalldir' ).exists()
