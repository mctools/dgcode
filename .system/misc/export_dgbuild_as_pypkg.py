#!/usr/bin/env python3

#Files not in a package which should be included anyway:

import pathlib
import shutil
import stat

def parse():
    #FIXME: Proper argparse?
    import sys
    _srcdir_val = pathlib.Path.home() / 'work' / 'dgcode_val'
    assert _srcdir_val.exists()

    class Args:
        srcdir = pathlib.Path(__file__).parent.parent.absolute().resolve()
        srcdir_val = _srcdir_val.absolute().resolve()
        targetdir = srcdir / 'exported_pypkg'
        targetdir_core = targetdir / 'core_pkg'
        targetdir_dgcode = targetdir / 'dgcode_pkg'
        force = '--force' in sys.argv[1:]
    return Args()

def create_metadata_files_core( tgtdir ):
    ( tgtdir / 'LICENSE' ).write_text('https://www.apache.org/licenses/LICENSE-2.0')
    descr='Simple build system multi-user and multi-project development focused on C++/Python projects.'
    ( tgtdir / 'README.md' ).write_text(descr)
    ( tgtdir / 'MANIFEST.in' ).write_text("""
recursive-include src/simplebuild/data/cmake *.cmake *.txt *.py
recursive-include src/simplebuild/data/pkgs-core LICENSE pkg.info *.cc *.hh *.icc *.c *.h *.f *.py
recursive-include src/simplebuild/data/pkgs-core/**/data *
recursive-include src/simplebuild/data/pkgs-core/**/scripts *
recursive-include src/simplebuild/data/pkgs-core_val LICENSE pkg.info *.cc *.hh *.icc *.c *.h *.f *.py
recursive-include src/simplebuild/data/pkgs-core_val/**/data *
recursive-include src/simplebuild/data/pkgs-core_val/**/scripts *
recursive-include src simplebuild.cfg

global-exclude *~* *\#* .*
global-exclude *.pyc *.pyo */__pycache__/* */.ruff_cache/*
global-exclude *.gch *.exe *.so *.dylib *.o *.a
global-exclude vgcore.* .DS_Store .vscode *.reflogupdate*.orig
""")

    ( tgtdir / 'pyproject.toml' ).write_text(f"""
[build-system]
requires = ["setuptools>=61.0","setuptools-git-versioning"]
build-backend = "setuptools.build_meta"

[tool.setuptools-git-versioning]
enabled = true

[project]
name = "simplebuild"
dynamic = ["version"] # Version based on git tag...
#... not static: version = "0.0.1"
authors = [
  {{ name="Thomas Kittelmann", email="thomas.kittelmann@ess.eu" }},
]
description = "{descr}"
readme = "README.md"
requires-python = ">=3.8"
classifiers = [
    "Programming Language :: Python :: 3",
    "License :: OSI Approved :: Apache Software License",
    "Operating System :: Unix",
]
dependencies = [
    #Very light-weight, not a big deal if we happen to install with both conda and pip:
    'tomli; python_version < "3.11"'
]

[project.optional-dependencies]
all = [
    #Non-trivial dependencies which is most likely already installed in conda
    #(if we actually make conda-packages, we can move these up to the
    #non-optional dependencies above):
   'pybind11 >= 2.10.4',
    'cmake >= 3.24.2'
]

[project.urls]
"Homepage" = "https://github.com/mctools/simplebuild"
"Bug Tracker" = "https://github.com/mctools/simplebuild/issues"

[project.scripts]
simplebuild2 = "simplebuild._cli_dgbuild:main"
unwrapped_simplebuild2 = "simplebuild._cli_dgbuild:unwrapped_main"
sbtests2 = "simplebuild._cli_dgtests:main"
sbenv2 = "simplebuild._cli_dgbuild:dgenv_main"
sbrun2 = "simplebuild._cli_dgbuild:dgrun_main"
""")

def create_metadata_files_dgcode( tgtdir ):
    ( tgtdir / 'LICENSE' ).write_text('https://www.apache.org/licenses/LICENSE-2.0')#FIXME: Actual file!
    ( tgtdir / 'README.md' ).write_text('Build system for the ESS-detector group coding framework.')
    #    ( tgtdir / 'setup.py' ).write_text('dummy')
    ( tgtdir / 'MANIFEST.in' ).write_text("""
recursive-include src/simplebuild-dgcode/data/pkgs LICENSE pkg.info *.cc *.hh *.icc *.c *.h *.f *.py
recursive-include src/simplebuild-dgcode/data/pkgs/**/data *
recursive-include src/simplebuild-dgcode/data/pkgs/**/scripts *
recursive-include src/simplebuild-dgcode/data/pkgs_val LICENSE pkg.info *.cc *.hh *.icc *.c *.h *.f *.py
recursive-include src/simplebuild-dgcode/data/pkgs_val/**/data *
recursive-include src/simplebuild-dgcode/data/pkgs_val/**/scripts *
recursive-include src simplebuild.cfg

global-exclude *~* *\#* .*
global-exclude *.pyc *.pyo */__pycache__/* */.ruff_cache/*
global-exclude *.gch *.exe *.so *.dylib *.o *.a
global-exclude vgcore.* .DS_Store .vscode *.reflogupdate*.orig
""")

    ( tgtdir / 'pyproject.toml' ).write_text("""
[build-system]
requires = ["setuptools>=61.0","setuptools-git-versioning"]
build-backend = "setuptools.build_meta"

[tool.setuptools-git-versioning]
enabled = true

[project]
name = "simplebuild-dgcode"
dynamic = ["version"] # Version based on git tag...
#... not static: version = "0.0.1"
authors = [
  { name="Thomas Kittelmann", email="thomas.kittelmann@ess.eu" },
]
description = "The Geant4-based coding framework originating in the ESS Detector Group, provided as a simplebuild bundle of packages."
readme = "README.md"
requires-python = ">=3.8"
classifiers = [
    "Programming Language :: Python :: 3",
    "License :: OSI Approved :: Apache Software License",
    "Operating System :: Unix",
]
dependencies = [
    'simplebuild'
]

[project.urls]
"Homepage" = "https://github.com/mctools/dgcode"
"Bug Tracker" = "https://github.com/mctools/dgcode/issues"
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
        if l.startswith('// DGBUILD-EXPORT-ONLY>>'):
            l = l[len('// DGBUILD-EXPORT-ONLY>>'):]
        res += '%s\n'%l
    return res

def is_backup_filename( name ):
    return '#' in name or '~' in name

def ignore_filename( name ):
    return not name or is_backup_filename(name) or name[0]=='.' or name.startswith('__pycache__') or name.endswith('.egg-info')

def create_pymodfiles( srcdir, tgtdir, ignorefct = None ):
    tgtdir.mkdir( parents = True )
    for f in sorted(srcdir.glob('*.py')):
        if ignorefct and ignorefct(f.name):
            print("WARNING: Ignoring %s"%f)
            continue
        ( tgtdir / f.name ).write_text( extract_file_content(f) )

def create_pymodfiles_dgcode( tgtdir ):
    tgtdir.mkdir( parents = True )
    ( tgtdir / '__init__.py' ).write_text('')
    ( tgtdir / 'simplebuild_bundle_list.py' ).write_text(
"""
def simplebuild_bundle_list():
    import pathlib
    datadir = ( pathlib.Path(__file__).absolute().parent / 'data' ).absolute().resolve()
    return [ datadir / 'pkgs' / 'simplebuild.cfg', datadir / 'pkgs_val' / 'simplebuild.cfg' ]
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
                if f.name not in ('ExtDep_NoSystemNCrystal.cmake', 'ExtDep_Boost.cmake'):
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
            if '~' in str(f) or '#' in str(f) or '.reflogupdate.' in f.name:
                print('WARNING: Ignoring file:',f)
                continue
            f_reltopkg = f.relative_to(pkgdir)
            print ('    -> ',f)
            tgtfile = tgtdir / pkgreldir / f_reltopkg
            tgtfile.parent.mkdir(parents=True,exist_ok=True)
            text_data = extract_file_content(f)
            if text_data is not None:
                tgtfile.write_text( text_data )
            else:
                print('WARNING: Transferring binary data file:',f)
                assert ( tgtfile.parent.name == 'data'
                         or ( tgtfile.parent.parent.name in ('MCPLTests','MCPLZLibTests')
                              and tgtfile.name in ('testtool_sysgz.log','testtool_nogz.log','testtool.log'))
                         or ( tgtfile.parent.parent.name=='MCPLPyTests' and tgtfile.name=='testpyd.log')
                        )
                tgtfile.write_bytes( f.read_bytes() )

            if tgtfile.parent.name=='scripts' and len(f_reltopkg.parts)==2 and not tgtfile.name.endswith('.log'):
                chmod_x( tgtfile )

def main():
    opt = parse()
    if opt.targetdir.exists():
        if not opt.force:
            raise SystemExit( 'Target-directory exists and --force was not provided: %s'%opt.targetdir )
        shutil.rmtree( opt.targetdir )
    print('Extracting to %s'%opt.targetdir)
    opt.targetdir.mkdir( parents=True )
    opt.targetdir_core.mkdir( parents=True )
    opt.targetdir_dgcode.mkdir( parents=True )
    create_metadata_files_core( opt.targetdir_core )
    create_metadata_files_dgcode( opt.targetdir_dgcode )
    destdir_mods = opt.targetdir_core / 'src' / 'simplebuild'
    destdir_mods_dgcode = opt.targetdir_dgcode / 'src' / 'simplebuild-dgcode'
    destdir_data = destdir_mods / 'data'
    destdir_data_dgcode = destdir_mods_dgcode / 'data'
    destdir_cmake = destdir_data / 'cmake'
    destdir_pkgs_core = destdir_data / 'pkgs-core'
    destdir_pkgs_coreval = destdir_data / 'pkgs-core_val'
    destdir_pkgs_dgcode = destdir_data_dgcode / 'pkgs'
    destdir_pkgs_dgcode_val = destdir_data_dgcode / 'pkgs_val'
    create_pymodfiles( opt.srcdir / 'mods' , destdir_mods,
                       ignorefct = ( lambda fname : fname in ['create_setup_file.py'] ),
                      )
    create_pymodfiles_dgcode( destdir_mods_dgcode )
    create_cmakefiles( opt.srcdir / 'cmakedetect' , destdir_cmake )

    create_frameworkpkgs( opt.srcdir_val, destdir_pkgs_coreval,
                          pkgfilter = (lambda pkgname : pkgname=='CoreTests') )
    create_frameworkpkgs( opt.srcdir_val, destdir_pkgs_dgcode_val,
                          pkgfilter = (lambda pkgname : pkgname!='CoreTests') )

    create_frameworkpkgs( opt.srcdir.parent / 'packages' / 'Framework', destdir_pkgs_core,
                          pkgfilter = (lambda pkgname : pkgname=='Core') )
    create_frameworkpkgs( opt.srcdir.parent / 'packages' / 'Framework', destdir_pkgs_dgcode,
                          pkgfilter = (lambda pkgname : pkgname not in ('Core',
                                                                        'DGBoost',
                                                                        'NCrystalBuiltin',
                                                                        #'NCrystalRel', <--TODO
                                                                        ) ) )

#simplebuild.cfg files:
    ( destdir_pkgs_core / 'simplebuild.cfg' ).write_text(
f"""[project]
  name = "core"
  env_paths = [ "PATH:<install>/bin:<install>/scripts",
                "PYTHONPATH:<install>/python" ]
[depend]
  search_path = [ "../{destdir_pkgs_coreval.name}" ]
""" )
    ( destdir_pkgs_coreval / 'simplebuild.cfg' ).write_text(
f"""[project]
  name = "core_val"
[depend]
  search_path = [ "../{destdir_pkgs_core.name}" ]
""" )

    ( destdir_pkgs_dgcode / 'simplebuild.cfg' ).write_text(
f"""[project]
  name = "dgcode"
  env_paths = [ "NCRYSTAL_DATA_PATH:<install>/data" ]
[depend]
  search_path= [ "../{destdir_pkgs_dgcode_val.name}" ]
""" )
    ( destdir_pkgs_dgcode_val / 'simplebuild.cfg' ).write_text(
f"""[project]
  name = "dgcode_val"
[depend]
  projects= [ "core_val", "dgcode" ]
  search_path= [ "../{destdir_pkgs_dgcode.name}" ]
""" )

    ( opt.targetdir_core / 'simplebuild_redirect.cfg' ).write_text(
f"""# Special simplebuild cfg file which allows one to use the pkgs in the contained
# bundles directly from a git clone of the repository rather than after
# installing as a python package, and simply by adding the root directory of the
# git clone to the depend.search_path list in a given simplebuild.cfg file. This
# exists to facilitate development work, and is not as such intended for
# end-users.

[special]
  redirect_search_path = [
  "./src/simplebuild/data/{destdir_pkgs_core.name}",
  "./src/simplebuild/data/{destdir_pkgs_coreval.name}",
  ]
""" )

    ( opt.targetdir_dgcode / 'simplebuild_redirect.cfg' ).write_text(
f"""# Special simplebuild cfg file which allows one to use the pkgs in the contained
# bundles directly from a git clone of the repository rather than after
# installing as a python package, and simply by adding the root directory of the
# git clone to the depend.search_path list in a given simplebuild.cfg file. This
# exists to facilitate development work, and is not as such intended for
# end-users.

[special]
  redirect_search_path = [
  "./src/simplebuild-dgcode/data/{destdir_pkgs_dgcode.name}",
  "./src/simplebuild-dgcode/data/{destdir_pkgs_dgcode_val.name}"
  ]
""" )


if __name__ == '__main__':
    main()
