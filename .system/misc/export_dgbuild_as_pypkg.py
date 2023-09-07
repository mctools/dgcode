#!/usr/bin/env python3

import pathlib
import shutil
import stat

def parse():
    #FIXME: Proper argparse?
    import sys
    class Args:
        srcdir = pathlib.Path(__file__).parent.parent.absolute().resolve()
        targetdir = srcdir / 'exported_pypkg'
        force = '--force' in sys.argv[1:]
    return Args()

def create_metadata_files( tgtdir ):
    ( tgtdir / 'LICENSE' ).write_text('https://www.apache.org/licenses/LICENSE-2.0')#FIXME: Actual file!
    ( tgtdir / 'README.md' ).write_text('Build system for the ESS-detector group coding framework.')
#    ( tgtdir / 'setup.py' ).write_text('dummy')
    ( tgtdir / 'pyproject.toml' ).write_text("""
#[build-system]
#requires = ["hatchling"]
#build-backend = "hatchling.build"

[build-system]
requires = ["setuptools>=61.0"]
build-backend = "setuptools.build_meta"

[project]
name = "ess_dgbuild"
version = "0.0.1"
authors = [
  { name="Thomas Kittelmann", email="thomas.kittelmann@ess.eu" },
]
description = "Build system for the ESS-detector group coding framework"
readme = "README.md"
requires-python = ">=3.8"
classifiers = [
    "Programming Language :: Python :: 3",
    "License :: OSI Approved :: Apache Software License",
    "Operating System :: Unix",
]

[project.scripts]
dgbuild2 = "ess_dgbuild_internals._cli_dgbuild:main"
dgtests2 = "ess_dgbuild_internals._cli_dgtests:main"

[project.urls]
"Homepage" = "https://github.com/mctools/dgcode"
"Bug Tracker" = "https://github.com/mctools/dgcode/issues"

""")

def extract_file_content( f ):
    res = ''
    for l in f.read_text().splitlines():
        if 'DGBUILD-NO-EXPORT' in l:
            continue
        if l.startswith('# DGBUILD-EXPORT-ONLY>>'):
            l = l[len('# DGBUILD-EXPORT-ONLY>>'):]
        res += '%s\n'%l
    return res

def is_backup_filename( name ):
    return '#' in name or '~' in name

def ignore_filename( name ):
    return not name or is_backup_filename(name) or name[0]=='.' or name.startswith('__pycache__') or name.endswith('.egg-info')

def create_pymodfiles( srcdir, tgtdir ):
    tgtdir.mkdir( parents = True )
    for f in sorted(srcdir.glob('*.py')):
        ( tgtdir / f.name ).write_text( extract_file_content(f) )

def create_cmakefiles( srcdir, tgtdir ):
    tgtdir.mkdir( parents = True )
    for f in sorted(srcdir.glob('*')):
        if ignore_filename(f.name):
            if not is_backup_filename(f.name):
                print("WARNING: Ignoring %s"%f)
            continue
        if f.is_dir():
            if f.name in ('optional','extras'):
                create_cmakefiles( f, tgtdir / f.name )
                continue
        else:
            if f.suffix in ('.cmake','.txt','.py'):
                ( tgtdir / f.name ).write_text( extract_file_content(f) )
                continue
        print("WARNING: Ignoring %s"%f)

pkgfile_patterns = ['LICENSE','pkg.info','data/*','scripts/*','python/*.py',
                    'pycpp_*/*.hh','pycpp_*/*.cc','pycpp_*/*.c','pycpp_*/*.h',
                    'app_*/*.hh', 'app_*/*.cc', 'app_*/*.h', 'app_*/*.c', 'app_*/*.f',
                    'libsrc/*.hh', 'libsrc/*.cc', 'libsrc/*.h', 'libsrc/*.c', 'libsrc/*.f',
                    'libinc/*.hh', 'libinc/*.h',
                    ]

def get_pkg_files( pkgdir ):
    files = set()
    for pattern in pkgfile_patterns:
        for f in pkgdir.glob(pattern):
            if not is_backup_filename(f.name):
                if f.name.startswith('dgboostpython_') and pkgdir.name=='DGBoost':
                    continue
                files.add( f.absolute().resolve() )
    return files

def chmod_x( path ):
    path.chmod( path.stat().st_mode | stat.S_IEXEC )

def create_frameworkpkgs( srcdir, tgtdir ):
    tgtdir.mkdir( parents = True )
    export_only = ['Core','G4Units']
    export_block = ['DGBoost']
    #Files not in a package which should be included anyway:
    extra_files = ['Framework/External/G4Units/LICENSE']

    for pkgcfg in srcdir.glob('**/pkg.info'):
        pkgdir = pkgcfg.parent
        pkgname = pkgdir.name
        if export_only is not None and not pkgname in export_only:
            continue
        if pkgname in export_block:
            continue
        pkgreldir = pkgdir.relative_to(srcdir)
        newpkgdir = tgtdir / pkgreldir
        print('Transferring files from: %s'%(pkgreldir))
        for f in sorted(get_pkg_files(pkgdir)):
            f_reltopkg = f.relative_to(pkgdir)
            print ('    -> ',f)
            tgtfile = tgtdir / pkgreldir / f_reltopkg
            tgtfile.parent.mkdir(parents=True,exist_ok=True)
            tgtfile.write_text( extract_file_content(f) )
            if tgtfile.parent.name=='scripts' and len(f_reltopkg.parts)==2:
                chmod_x( tgtfile )

def main():
    opt = parse()
    if opt.targetdir.exists():
        if not opt.force:
            raise SystemExit( 'Target-directory exists and --force was not provided: %s'%opt.targetdir )
        shutil.rmtree( opt.targetdir )
    print('Extracting to %s'%opt.targetdir)
    opt.targetdir.mkdir( parents=True )
    create_metadata_files( opt.targetdir )
    destdir_mods = opt.targetdir / 'src' / 'ess_dgbuild_internals'
    destdir_data = destdir_mods / 'data'
    destdir_cmake = destdir_data / 'cmake'
    destdir_pkgs = destdir_data / 'frameworkpkgs'
    create_pymodfiles( opt.srcdir / 'mods' , destdir_mods )
    create_cmakefiles( opt.srcdir / 'cmakedetect' , destdir_cmake )
    create_frameworkpkgs( opt.srcdir.parent / 'packages' / 'Framework', destdir_pkgs )

if __name__ == '__main__':
    main()
