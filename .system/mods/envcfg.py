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

    #These are the basic ones:
    build_dir_resolved = _query('DGCODE_BUILD_DIR_RESOLVED')
    install_dir_resolved = _query('DGCODE_INSTALL_DIR_RESOLVED')
    projects_dir = _query('DGCODE_PROJECTS_DIR')
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


def _find_plugins():
    import importlib
    import pkgutil
    possible_plugins = {
        name: importlib.import_module(name)
        for finder, name, ispkg
        in pkgutil.iter_modules()
        if name.startswith('ess_dgbuild_')
    }
    plugins = {}
    for name,mod in sorted(possible_plugins.items()):
        if hasattr(mod,'dgbuild_bundle_name'):
            name = getattr(mod,'dgbuild_bundle_name')()
            if name in plugins:
                warnings.warn('Multiple plugins named "%s" available. Keeping only the first one.'%name)
                continue
            plugins[name] = dict(
                name = name,
                pkgroot = getattr(mod,'dgbuild_bundle_pkgroot')() if hasattr(mod,'dgbuild_bundle_pkgroot') else None,
                extdeps = getattr(mod,'dgbuild_bundle_extdeps')() if hasattr(mod,'dgbuild_bundle_extdeps') else None,
            )
    return plugins

def _newcfg():
    from .readcfg import cfg
    import pathlib

    the_extra_pkg_path = []
    if cfg.needs_bundles:
        plugins = _find_plugins()
        for b in cfg.needs_bundles:
            p = plugins.get(b)
            if not p:
                from . import error
                error.error(f'Bundle not found in current environment: {b}')
            if p['pkgroot']:
                pr = pathlib.Path(p['pkgroot']).absolute().resolve()
                if not pr in the_extra_pkg_path:
                    the_extra_pkg_path.append(pr)

    class EnvCfg:
        #These are the basic ones:
        build_dir_resolved = cfg.build_cachedir / 'bld'
        install_dir_resolved = cfg.build_cachedir / 'install'
        projects_dir = cfg.project_topdir
        extra_pkg_path = ':'.join(str(e) for e in the_extra_pkg_path)#fixme: keep at Path objects.
        enable_projects_pkg_selection_flag = False#fixme: we could allow this?

        #These are used in the context of conda installs:
        conda_prefix =  _query('CONDA_PREFIX')
        cmake_args =  _query('CMAKE_ARGS')

        #These are most likely almost never used by anyone (thus keeping as env vars for now!):
        color_fix_code = _query('DGCODE_COLOR_FIX')
        allow_sys_dev =  _query('DGCODE_ALLOWSYSDEV',boolean=True)

        # NOTE: backend.py also checks environment variables to check if something
        # changed needing an automatic cmake reconf. We provide a good base list
        # here:

        #FIXME: Too large list for this mode!:
        reconf_env_vars = [
        #First all of the above except DGCODE_ALLOWSYSDEV:
            'DGCODE_PROJECTS_DIR', 'DGCODE_INSTALL_DIR_RESOLVED','DGCODE_BUILD_DIR_RESOLVED',
            'DGCODE_EXTRA_PKG_PATH', 'DGCODE_ENABLE_PROJECTS_PKG_SELECTION_FLAG','DGCODE_COLOR_FIX',
            'CONDA_PREFIX','CMAKE_ARGS','PYTHONPATH',
            #Then some more used in our cmake modules (but not most of those marked for
            #reconf inside the ExtDep_xxx.cmake files):
            'DGCODE_USECONDABOOSTPYTHON'
        ]
    return EnvCfg()

var = EnvCfgClassic()# DGBUILD-NO-EXPORT
# DGBUILD-EXPORT-ONLY>>var = _newcfg()

del _query
