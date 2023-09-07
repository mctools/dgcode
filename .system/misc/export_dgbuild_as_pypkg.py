#!/usr/bin/env python3

import pathlib
import shutil

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

[project.urls]
"Homepage" = "https://github.com/mctools/dgcode"
"Bug Tracker" = "https://github.com/mctools/dgcode/issues"

""")

def extract_py_file_content( f ):
    res = ''
    for l in f.read_text().splitlines():
        if 'DGBUILD-NO-EXPORT' in l:
            continue
        if l.startswith('# DGBUILD-EXPORT-ONLY>>'):
            l = l[len('# DGBUILD-EXPORT-ONLY>>'):]
        res += '%s\n'%l
    return res

def create_pymodfiles( srcdir, tgtdir ):
    outsrc = tgtdir / 'src'
    outmod = tgtdir / 'src' / 'ess_dgbuild_internals'
    outmod.mkdir( parents = True )
    for f in sorted(srcdir.glob('*.py')):
        ( outmod / f.name ).write_text( extract_py_file_content(f) )

def main():
    opt = parse()
    if opt.targetdir.exists():
        if not opt.force:
            raise SystemExit( 'Target-directory exists and --force was not provided: %s'%opt.targetdir )
        shutil.rmtree( opt.targetdir )
    print('Extracting to %s'%opt.targetdir)
    opt.targetdir.mkdir( parents=True )
    create_metadata_files( opt.targetdir )
    create_pymodfiles( opt.srcdir / 'mods' , opt.targetdir )

if __name__ == '__main__':
    main()
