from .singlecfg import SingleCfg
from . import error

def locate_master_cfg_file():
    import pathlib
    import os
    p = os.environ.get('DGBUILD_CFG')
    if p:
        p = pathlib.Path(p).expanduser()
        if not p.exists():
            error.error('DGBUILD_CFG was set to a non-existing directory or file')
        if p.is_dir():
            p = p / 'dgbuild.cfg'
            if not p.exists():
                error.error('DGBUILD_CFG was set to a directory'
                            ' with no dgbuild.cfg file in it')
        else:
            if not p.exists():
                error.error('DGBUILD_CFG was set to non-existing file')
        if not p.is_absolute():
            error.error('DGBUILD_CFG must be set to an absolute path')
        return p
    p = pathlib.Path('.').absolute()#NB: NOT .resolve() on purpose!
    f = p / 'dgbuild.cfg'
    if f.exists():
        return f
    for p in sorted(p.parents,reverse=True):
        f = p / 'dgbuild.cfg'
        if f.exists():
            return f

def load_core_cfg():
    import pathlib
    class CoreBundleCfg:
        def dgbuild_srcdescr_override( self ):
            return 'dgbuildcore bundle (builtin)'
        def dgbuild_bundle_name( self ):
            return 'dgbuildcore'
        def dgbuild_bundle_pkgroot( self ):
            return ( pathlib.Path(__file__).absolute().parent
                     / 'data' / 'pkgs' / 'Framework' ).absolute().resolve()
        def dgbuild_bundle_envpaths( self ):
            return [ 'PATH:<install>/bin:<install>/scripts',
                     'PYTHONPATH:<install>/python' ]
    return SingleCfg.create_from_object_methods(CoreBundleCfg(),ignore_build=True)

class CfgBuilder:

    """Class which, based on an initial "master" dgbuild.cfg, can locate and
    extract other dgbuild cfg's, and provide a combined configuration object.

    The "build" section (build mode, ncpu, pkgfilter, etc.) is taken straight
    from the master config, while the other sections are based on a combination
    of data in the master_cfg and any cfg's of the dependencies.

    """

    def __init__(self, master_cfg : SingleCfg ):
        #Input:
        #Take build settings straight from master_cfg:
        self.__build_mode = master_cfg.build_mode
        self.__build_njobs = master_cfg.build_njobs
        self.__build_cachedir = master_cfg.build_cachedir
        self.__build_pkg_filter = master_cfg.build_pkg_filter
        self.__build_extdep_ignore = master_cfg.build_extdep_ignore

        #Build up everything else recursively, starting from the master_cfg:
        self.__pkg_path = []#result 1
        self.__env_paths = {}#result 2
        self.__cfg_search_path_already_considered = set()
        self.__available_unused_cfgs = []#only needed during build up
        self.__available_unused_cfgs.append( load_core_cfg() )#dgbuildcore bundle always available
        self.__cfg_names_used = set()
        self.__cfg_names_missing = set([ master_cfg.project_name ])
        self.__use_cfg( master_cfg, is_top_level = True )
        if self.__cfg_names_missing:
            #Only consider python plugins if there is anything we did not find
            #already in the directly requested search paths (that way a dev
            #environment with everything checked out will be unaffected by what
            #plugins happen to be installed in the environment):
            self.__consider_python_plugins()
        if self.__cfg_names_missing:
            _p='", "'.join(self.__cfg_names_missing)
            _s = 's' if len(self.__cfg_names_missing)>2 else ''
            error.error('Could not find dependent project%s: "%s"'%(_s,_p))
        self.__pkg_path = tuple( self.__pkg_path )
        #self.__env_paths = tuple( self.__env_paths )
        del self.__available_unused_cfgs

    @property
    def build_mode( self ):
        return self.__build_mode

    @property
    def build_njobs( self ):
        return self.__build_njobs

    @property
    def build_cachedir( self ):
        return self.__build_cachedir

    @property
    def build_pkg_filter( self ):
        return self.__build_pkg_filter

    @property
    def build_extdep_ignore( self ):
        return self.__build_extdep_ignore

    @property
    def pkg_path(self):
        return self.__pkg_path

    @property
    def env_paths(self):
        return self.__env_paths

    def __use_cfg( self, cfg : SingleCfg, is_top_level = False ):
        if not cfg.project_name in self.__cfg_names_missing:
            #We do not actually need this cfg!
            return
        self.__cfg_names_missing.remove( cfg.project_name )
        self.__cfg_names_used.add( cfg.project_name )
        #Add dependencies and cfgs available in search paths:
        for cfgname in cfg.depend_projects:
            if not cfgname in self.__cfg_names_used:
                self.__cfg_names_missing.add( cfgname )
        for sp in cfg.depend_search_path:
            if not sp.exists():
                error.error
            if sp in self.__cfg_search_path_already_considered:
                continue
            assert sp.is_file()
            self.__cfg_search_path_already_considered.add( sp )
            depcfg = SingleCfg.create_from_toml_file(
                sp,
                ignore_build = not is_top_level
            )
            self.__available_unused_cfgs.append( depcfg )

        #Add actual pkg-dirs and env-path requests from cfg:
        if not cfg.project_pkg_root in self.__pkg_path:
            self.__pkg_path.append( cfg.project_pkg_root )
        for pathvar,contents in cfg.project_env_paths.items():
            if not pathvar in self.__env_paths:
                self.__env_paths[pathvar] = set()
            self.__env_paths[pathvar].update( contents )

        #Use cfgs we found as appropriate:
        self.__search_available_cfgs()

    def __search_available_cfgs( self ):
        if not self.__cfg_names_missing:
            return
        to_use_idx, to_use_cfg = set(), []
        for i,ucfg in enumerate(self.__available_unused_cfgs):
            if ucfg.project_name in self.__cfg_names_missing:
                to_use_cfg.append( ucfg )
                to_use_idx.add( i )
        if to_use_cfg:
            for i in sorted(to_use_idx,reverse=True):
                del self.__available_unused_cfgs[i]
            for ucfg in to_use_cfg:
                self.__use_cfg( ucfg )

    def __consider_python_plugins( self ):
        #First, find all available python plugins:
        import importlib
        import pkgutil
        possible_pyplugins = set(
            name
            for finder, name, ispkg
            in pkgutil.iter_modules()
            if ( name.startswith('ess_dgbuild_') or name.startswith('dgbuild_') )
        )

        #Load their cfgs:
        for name in sorted(possible_pyplugins):
            try:
                mod = importlib.import_module(f'{name}.dgbuild_bundle_info')
            except ModuleNotFoundError:
                continue
            if not SingleCfg.is_plugin_object( mod ):
                continue
            srcdescr = 'Python module %s'%name
            cfg = SingleCfg.create_from_object_methods( mod,
                                                        srcdescr = srcdescr,
                                                        ignore_build = True )
            self.__available_unused_cfgs.append( cfg )

        #Use cfgs we found as appropriate:
        self.__search_available_cfgs()
