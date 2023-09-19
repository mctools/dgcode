#!/usr/bin/env python3

#Files not in a package which should be included anyway:

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
    ( tgtdir / 'MANIFEST.in' ).write_text("""
recursive-include src/ess_dgbuild_internals/data/cmake *.cmake *.txt *.py
recursive-include src/ess_dgbuild_internals/data/pkgs/Framework LICENSE pkg.info *.cc *.hh *.icc *.c *.h *.f *.py
recursive-include src/ess_dgbuild_internals/data/pkgs/Framework/**/data *
recursive-include src/ess_dgbuild_internals/data/pkgs/Framework/**/scripts *
recursive-include src/ess_dgbuild_g4framework/data/pkgs LICENSE pkg.info *.cc *.hh *.icc *.c *.h *.f *.py
recursive-include src/ess_dgbuild_g4framework/data/pkgs/**/data *
recursive-include src/ess_dgbuild_g4framework/data/pkgs/**/scripts *

global-exclude *~* *\#* .*
global-exclude *.pyc *.pyo */__pycache__/* */.ruff_cache/*
global-exclude *.gch *.exe *.so *.dylib *.o *.a
global-exclude vgcore.* .DS_Store .vscode *.reflogupdate*.orig
""")

    ( tgtdir / 'pyproject.toml' ).write_text("""
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

[project.urls]
"Homepage" = "https://github.com/mctools/dgcode"
"Bug Tracker" = "https://github.com/mctools/dgcode/issues"

[project.scripts]
dgbuild2 = "ess_dgbuild_internals._cli_dgbuild:main"
dgtests2 = "ess_dgbuild_internals._cli_dgtests:main"
""")

def extract_file_content( f ):
    try:
        text_data = f.read_text()
    except UnicodeDecodeError:
        return None
    res = ''
    for l in text_data.splitlines():
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

def create_pymodfiles_g4( tgtdir ):
    tgtdir.mkdir( parents = True )
    ( tgtdir / '__init__.py' ).write_text('')
    ( tgtdir / 'dgbuild_bundle_info.py' ).write_text(
"""
import pathlib
def dgbuild_bundle_name():
    return 'g4framework'

def dgbuild_bundle_pkgroot():
    return ( pathlib.Path(__file__).parent / 'data' / 'pkgs' ).absolute().resolve()
"""
    )

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
                if f.name !=  'ExtDep_NoSystemNCrystal.cmake':
                    ( tgtdir / f.name ).write_text( extract_file_content(f) )
                    continue
        print("WARNING: Ignoring %s"%f)

pkgfile_patterns = ['LICENSE','pkg.info','data/*','scripts/*','python/*.py',
                    'pycpp_*/*.hh','pycpp_*/*.cc', 'pycpp_*/*.icc','pycpp_*/*.c','pycpp_*/*.h',
                    'app_*/*.hh', 'app_*/*.icc', 'app_*/*.cc', 'app_*/*.h', 'app_*/*.c', 'app_*/*.f',
                    'libsrc/*.hh', 'libsrc/*.icc', 'libsrc/*.cc', 'libsrc/*.h', 'libsrc/*.c', 'libsrc/*.f',
                    'libinc/*.hh', 'libinc/*.icc', 'libinc/*.h',
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

def create_frameworkpkgs( srcdir, tgtdir, pkgfilter ):
    tgtdir.mkdir( parents = True )

    for pkgcfg in srcdir.glob('**/pkg.info'):
        pkgdir = pkgcfg.parent
        pkgname = pkgdir.name
        if not pkgfilter(pkgname):
            continue
        pkgreldir = pkgdir.relative_to(srcdir)
        newpkgdir = tgtdir / pkgreldir
        print('Transferring files from: %s'%(pkgreldir))
        for f in sorted(get_pkg_files(pkgdir)):
            f_reltopkg = f.relative_to(pkgdir)
            print ('    -> ',f)
            tgtfile = tgtdir / pkgreldir / f_reltopkg
            tgtfile.parent.mkdir(parents=True,exist_ok=True)
            text_data = extract_file_content(f)
            if text_data is not None:
                tgtfile.write_text( text_data )
            else:
                assert tgtfile.parent.name == 'data'
                print('WARNING: Transferring binary data file:',f)
                tgtfile.write_bytes( f.read_bytes() )

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
    destdir_modsg4 = opt.targetdir / 'src' / 'ess_dgbuild_g4framework'
    destdir_data = destdir_mods / 'data'
    destdir_datag4 = destdir_modsg4 / 'data'
    destdir_cmake = destdir_data / 'cmake'
    destdir_pkgs = destdir_data / 'pkgs/Framework'
    destdir_pkgsg4 = destdir_datag4 / 'pkgs'
    create_pymodfiles( opt.srcdir / 'mods' , destdir_mods )
    create_pymodfiles_g4( destdir_modsg4 )
    create_cmakefiles( opt.srcdir / 'cmakedetect' , destdir_cmake )
    create_frameworkpkgs( opt.srcdir.parent / 'packages' / 'Framework', destdir_pkgs,
                          pkgfilter = (lambda pkgname : pkgname=='Core') )
    create_frameworkpkgs( opt.srcdir.parent / 'packages' / 'Framework', destdir_pkgsg4,
                          pkgfilter = (lambda pkgname : pkgname not in ('Core','DGBoost','NCrystalBuiltin') ) )

if __name__ == '__main__':
    main()
