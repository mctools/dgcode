# To keep things clean and allow for future migrations to different cfg systems,
# this file should be the ONLY place we query environment variables!!

def _query(n, *, boolean=False):
    import os
    _ = os.environ.get(n)
    if boolean:
        return (_ or '').lower() in ['true', 'on', 'yes', '1']
    return _

class EnvCfgClassic:
    #Important note: The code USING these variables might emit error messages
    #mentioning the name of the environment variables. Thus, any changes here
    #should be reflected in the code using them as well.

    legacy_mode = True

    #These are the basic ones:
    import pathlib
    build_dir_resolved = pathlib.Path(_query('DGCODE_BUILD_DIR_RESOLVED')).resolve().absolute()
    install_dir_resolved = pathlib.Path(_query('DGCODE_INSTALL_DIR_RESOLVED')).resolve().absolute()
    projects_dir = pathlib.Path(_query('DGCODE_PROJECTS_DIR')).resolve().absolute()
    extra_pkg_path = _query('DGCODE_EXTRA_PKG_PATH')
    enable_projects_pkg_selection_flag = _query('DGCODE_ENABLE_PROJECTS_PKG_SELECTION_FLAG',boolean=True)

    #These are used in the context of conda installs:
    conda_prefix =  _query('CONDA_PREFIX')
    cmake_args =  _query('CMAKE_ARGS')

    #These are most likely almost never used by anyone:
    color_fix_code = _query('DGCODE_COLOR_FIX')
    allow_sys_dev =  _query('DGCODE_ALLOWSYSDEV',boolean=True)

    # NOTE: backend.py also checks environment variables to check if something
    # changed needing an automatic cmake reconf. We provide a good base list
    # here:
    reconf_env_vars = [
        #First all of the above except DGCODE_ALLOWSYSDEV:
        'DGCODE_PROJECTS_DIR', 'DGCODE_INSTALL_DIR_RESOLVED','DGCODE_BUILD_DIR_RESOLVED',
        'DGCODE_EXTRA_PKG_PATH', 'DGCODE_ENABLE_PROJECTS_PKG_SELECTION_FLAG','DGCODE_COLOR_FIX',
        'CONDA_PREFIX','CMAKE_ARGS',
        #Then some more used in our cmake modules (but not most of those marked for
        #reconf inside the ExtDep_xxx.cmake files):
        'DGCODE_USECONDABOOSTPYTHON'
    ]

    env_paths = {}

def _newcfg():
    from .cfgbuilder import locate_master_cfg_file, CfgBuilder
    from .singlecfg import SingleCfg
    from .pkgfilter import PkgFilter
    master_cfg_file = locate_master_cfg_file()

    if not master_cfg_file or not master_cfg_file.is_file():
        from . import error
        error.error('In order to continue, please step into a directory tree with a dgbuild.cfg file at its root.')

    assert master_cfg_file.is_file()

    master_cfg = SingleCfg.create_from_toml_file( master_cfg_file )
    cfg = CfgBuilder( master_cfg )
    pkgfilterobj = PkgFilter( cfg.build_pkg_filter )

    class EnvCfg:

        legacy_mode = False

        #These are the basic ones:
        build_dir_resolved = cfg.build_cachedir / 'bld'
        install_dir_resolved = cfg.build_cachedir / 'install'
        projects_dir = master_cfg.project_pkg_root #FIXME: Is this ok?
        extra_pkg_path = ':'.join(str(e) for e in cfg.pkg_path)#fixme: keep at Path objects.
        extra_pkg_path_list = cfg.pkg_path#New style!
        enable_projects_pkg_selection_flag = False#fixme: we could allow this?
        pkg_filter = pkgfilterobj#New style!

        #These are used in the context of conda installs:
        conda_prefix =  _query('CONDA_PREFIX')
        cmake_args =  _query('CMAKE_ARGS')

        #These are most likely almost never used by anyone (thus keeping as env vars for now!):
        color_fix_code = _query('DGCODE_COLOR_FIX')
        allow_sys_dev =  _query('DGCODE_ALLOWSYSDEV',boolean=True)

        # NOTE: backend.py also checks environment variables to check if something
        # changed needing an automatic cmake reconf. We provide a good base list
        # here:

        reconf_env_vars = [
            #All of the above except DGCODE_ALLOWSYSDEV:
            'PATH','DGCODE_COLOR_FIX','CONDA_PREFIX','CMAKE_ARGS','PYTHONPATH',
            #Then some more used in our cmake modules (but not those marked for
            #reconf inside the optional/ExtDep_xxx.cmake files):
            'DGCODE_USECONDABOOSTPYTHON',
            #Also this one of course:
            'DGBUILD_CFG',
        ]

        env_paths = cfg.env_paths

    return EnvCfg()

var = EnvCfgClassic()# DGBUILD-NO-EXPORT
# DGBUILD-EXPORT-ONLY>>var = _newcfg()
