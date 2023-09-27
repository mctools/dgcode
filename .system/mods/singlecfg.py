__all__ = ['SingleCfg']

import pathlib
import types
import typing
import re
from . import error

class SingleCfg:

    """Class which contains decoded configuration from a single source (a
    dgbuild.cfg file, a python module, etc.)."""

    @classmethod
    def create_from_toml_data( cls, data : str, srcdescr = None, default_dir =  None, ignore_build : bool = False ):
        cfgdict = decode_toml_textdata_to_dict( data )
        return cls.__create_from_cfgdict( cfgdict,
                                          srcdescr = srcdescr or 'TOML data',
                                          default_dir = default_dir,
                                          ignore_build = ignore_build )

    @classmethod
    def create_from_toml_file( cls, path : pathlib.Path, ignore_build : bool = False ):
        try:
            textdata = path.read_text()
        except UnicodeDecodeError:
            error.error(f'Not a text-file: {path}')
        cfgdict = decode_toml_textdata_to_dict( textdata, path )
        return cls.__create_from_cfgdict( cfgdict, str(path), path.parent, ignore_build = ignore_build )

    @classmethod
    def is_plugin_object( cls, obj ):
        return hasattr( obj, 'dgbuild_bundle_toml_cfg' ) or hasattr( obj, 'dgbuild_bundle_name' )

    @classmethod
    def create_from_object_methods( cls, obj,
                                    srcdescr = None,
                                    default_dir : pathlib.Path | None = None,
                                    ignore_build : bool = False ):
        if hasattr(obj, 'dgbuild_default_dir'):
            p=pathlib.Path( obj.dgbuild_default_dir() ).expanduser()
            if p.is_dir():
                default_dir = p

        if default_dir is None and hasattr(obj, '__file__'):
            p=pathlib.Path(obj.__file__).parent
            if p.is_dir():
                default_dir = p

        if srcdescr is None:
            if hasattr( obj, 'dgbuild_srcdescr_override' ):
                srcdescr = obj.dgbuild_srcdescr_override()
            elif hasattr(obj,'__name__'):
                if isinstance(obj,types.ModuleType):
                    srcdescr = f'Python module {obj.__name__}'
                else:
                    srcdescr = f'Python object ({obj.__name__})'
            else:
                    srcdescr = 'anonymous Python object'

        if hasattr( obj, 'dgbuild_bundle_toml_cfg' ):
            tomldata = obj.dgbuild_bundle_toml_cfg()
            return cls.create_from_toml_data( tomldata,
                                              srcdescr = srcdescr,
                                              default_dir = default_dir,
                                              ignore_build = ignore_build )

        if not hasattr( obj, 'dgbuild_bundle_pkgroot' ):
            error.error(f'dgbuild Python plugin ({srcdescr}) is missing an dgbuild_bundle_pkgroot function')
        pkg_root = obj.dgbuild_bundle_pkgroot()
        if not isinstance(pkg_root,pathlib.Path) and not isinstance(pkg_root,str):
            error.error(f'dgbuild Python plugin ({srcdescr}) got invalid type from dgbuild_bundle_pkgroot'
                        ' function (must be str or pathlib.Path object)')

        env_paths_fct = getattr( obj, 'dgbuild_bundle_envpaths', None )
        if env_paths_fct:
            env_paths = env_paths_fct()
            if not isinstance( env_paths, list ) or not all( isinstance(e,str) for e in env_paths ):
                error.error(f'dgbuild Python plugin ({srcdescr}) got invalid type'
                            ' from dgbuild_bundle_envpaths function (must be list of str objects)')
        cfgdict = dict( project = dict( name = obj.dgbuild_bundle_name(),
                                        pkg_root = pkg_root,
                                        env_paths = env_paths,
                                        #extdeps = ( obj.dgbuild_bundle_extdeps()
                                        #            if hasattr(obj,'dgbuild_bundle_extdeps')
                                        #            else None ),
                                       )
                        )
        return cls.__create_from_cfgdict( cfgdict,
                                          srcdescr = srcdescr,
                                          default_dir = default_dir,
                                          ignore_build = ignore_build)

    @classmethod
    def __create_from_cfgdict( cls, cfgdict : dict, srcdescr : str, default_dir = None, ignore_build : bool = False ):
        o = cls.__create_empty()
        decode_with_schema_and_apply_result_to_obj( cfgdict, o,
                                                    defaultdir = default_dir,
                                                    srcdescr = srcdescr,
                                                    ignore_build = ignore_build )

        if o.project_name != 'dgbuildcore' and 'dgbuildcore' not in o.depend_projects:
            #Always add the core project as a dependency:
            o.depend_projects = list(e for e in o.depend_projects) + ['dgbuildcore']

        o._is_locked = True
        return o

    def __setattr__(self,name,*args,**kwargs):
        if getattr(self,'_is_locked',False):
            error.error(f'Trying to set attribute "{name}" of locked SingleCfg object.')
        object.__setattr__(self,name,*args,**kwargs)

    @classmethod
    def __create_empty( cls ):
        return cls( _private_construct_key = cls.__construct_key )

    __construct_key = id('dummy')#cheap "random" secret integer
    def __init__(self, *, _private_construct_key ):
        self._is_locked = False
        if _private_construct_key != self.__construct_key:
            raise TypeError("Direct initialisation disallowed")

#######################################################################################################

_cache_tomllib = [None]
def import_tomllib():
    if _cache_tomllib[0] is None:
        try:
            import tomllib
        except ModuleNotFoundError:
            try:
                import tomli as tomllib
            except ModuleNotFoundError:
                raise SystemExit('Required toml functionality is not found. Either use '
                                 'python 3.11+ or install tomli'
                                 ' ("conda install -c conda-forge tomli" or "pip install tomli") ')
        _cache_tomllib[0] = tomllib
    return _cache_tomllib[0]

class TOMLSchemaDecodeContext(typing.Protocol):
    @property
    def src_descr(self) -> str:
        return ''
    @property
    def default_dir(self) -> pathlib.Path | None:
        return None
    @property
    def item_name(self) -> str:
        return ''

_reexp_valid_identifier = r'^[A-Za-z][A-Za-z0-9_]*$'
_reobj_valid_identifier = re.compile(_reexp_valid_identifier)
def _is_valid_identifier( s ):
    return s and isinstance(s,str) and _reobj_valid_identifier.search(s) is not None

def _generate_toml_schema():

    def decode_nonempty_str( ctx : TOMLSchemaDecodeContext, item ):
        if not isinstance(item,str) or not str:
            error.error(f'Invalid value "{item}" for item {ctx.item_name} (expected non-empty string) in {ctx.src_descr}')
        return item

    def decode_valid_identifier_string( ctx : TOMLSchemaDecodeContext, item ):
        if not _is_valid_identifier(item):
            error.error(f'Invalid value "{item}" for item {ctx.item_name} (expected string matching {_reexp_valid_identifier}) in {ctx.src_descr}')
        return item

    def decode_is_list_of_valid_identifier_string( ctx : TOMLSchemaDecodeContext, item ):
        if not isinstance( item, list ):
            error.error(f'Invalid value "{item}" for item {ctx.item_name} (expected list) in {ctx.src_descr}')
        for i,k in enumerate(item):
            if not _is_valid_identifier(k):
                error.error(f'Invalid value of list entry #{i+1} in item {ctx.item_name} (expected string matching {_reexp_valid_identifier}) in {ctx.src_descr}')
        return tuple(item)

    def decode_is_env_paths( ctx : TOMLSchemaDecodeContext, item ):
        if not isinstance( item, list ):
            error.error(f'Invalid value "{item}" for item {ctx.item_name} (expected list) in {ctx.src_descr}')
        res = {}
        for idx,e in enumerate(item):
            parts = list(x.strip() for x in e.split(':<install>/') if x.strip())
            if not len(parts)>=2:
                error.error(f'Invalid value "{e}" for list entry #{idx+1} in item {ctx.item_name} (expected format like "VARNAME:<install>/SUBDIRNAME" but got "{e}") in {ctx.src_descr}')
            varname, install_subdirnames = parts[0],parts[1:]
            if not varname in res:
                res[varname] = set()
            res[varname].update(install_subdirnames)
        return res

    def decode_is_list_of_nonempty_str( ctx : TOMLSchemaDecodeContext, item ):
        if not isinstance( item, list ):
            error.error(f'Invalid value "{item}" for item {ctx.item_name} (expected list) in {ctx.src_descr}')
        for i,k in enumerate(item):
            if not isinstance(k,str) or not k:
                error.error(f'Invalid value of list entry #{i+1} in item {ctx.item_name} (expected non-empty string) in {ctx.src_descr}')
        return tuple(item)

    def decode_is_list_of_paths( ctx : TOMLSchemaDecodeContext, item ):
        if not isinstance( item, list ):
            error.error(f'Invalid value "{item}" for item {ctx.item_name} (expected list) in {ctx.src_descr}')
        res=[]
        for i,k in enumerate(item):
            if not isinstance(k,str) or not k:
                error.error(f'Invalid value of list entry #{i+1} in item {ctx.item_name} (expected non-empty string) in {ctx.src_descr}')
            p = pathlib.Path(k).expanduser()
            if not p.exists():
                error.error(f'Non-existing path "{p}" in list entry #{i+1} in item {ctx.item_name} in {ctx.src_descr}')
            if p.is_dir():
                #Unless the filename is specified directly, add the default name:
                p = ( p / 'dgbuild.cfg' )
                if not p.exists():
                    error.error(f'Missing file "{p}" in list entry #{i+1} in item {ctx.item_name} in {ctx.src_descr}')
            res.append(p.absolute().resolve())
        return tuple(res)

    def decode_str_enum( ctx : TOMLSchemaDecodeContext, item, optionlist ):
        item = decode_nonempty_str( ctx, item )
        if item not in optionlist:
            error.error(f'Invalid option "{item}" for item {ctx.item_name} (must be one of {",".join(optionlist)}) in {ctx.src_descr}')
        return item

    def decode_nonneg_int( ctx : TOMLSchemaDecodeContext, item ):
        if not isinstance(item,int):
            error.error(f'Invalid option "{item}" for item {ctx.item_name} (must be a non-negative integer) in {ctx.src_descr}')
        return item

    def decode_dir( ctx : TOMLSchemaDecodeContext, item ):
        if isinstance( item, pathlib.Path ):
            #special case, for simple python plugins which already returns Path's rather than str's.
            p = item
            item = str(item)
        else:
            item = decode_nonempty_str( ctx, item )
            p = pathlib.Path(item).expanduser()
        if not p.is_absolute():
            if ctx.default_dir is None:
                error.error(f'Invalid option "{item}" for item {ctx.item_name} (relative paths can only'
                            f' be used in a context where the "root" dir is apparent) in {ctx.src_descr}')
            p = ctx.default_dir / p
        return p.absolute().resolve()

    return dict( project   = dict( name        = (decode_valid_identifier_string,None),
                                   pkg_root      = (decode_dir, '.'),
                                   env_paths   = (decode_is_env_paths,[]),
                                  ),
                 depend     = dict( projects     = (decode_is_list_of_nonempty_str,[]),
                                    search_path = (decode_is_list_of_paths,[])
                                  ),
                 build     = dict( mode = ( lambda a,b : decode_str_enum(a,b,('debug','release')), 'release' ),
                                   njobs = (decode_nonneg_int, 0),
                                   cachedir = (decode_dir, './dgbuild_cache'),
                                   pkg_filter = (decode_is_list_of_nonempty_str,[]),
                                   #^^^
                                   #Starting with all packages available,
                                   #filters are matched in order, removing those
                                   #not matching (the matching is inverted if a
                                   #filter is prefixed with a "!" character.
                                   #
                                   #Matches which will exclude rather than
                                   #filter simply prefixed with !, entries with
                                   #'/' in them are matched on relative
                                   #directory to their pkg_root, otherwise just
                                   #on the name.
                                   #
                                   #To use regular expressions rather than shell globbing (fnmatch), prefix the filter with "RE::"

                                   extdep_ignore = (decode_is_list_of_valid_identifier_string,[]),
                                  ),
                )

_cache_toml_scheme = [None]
def get_toml_schema():
    if _cache_toml_scheme[0] is None:
        _cache_toml_scheme[0] = _generate_toml_schema()
    return _cache_toml_scheme[0]

def read_text_file( f ):
    try:
        data = f.read_text()
    except UnicodeDecodeError:
        error.error(f'Not a text-file: {f}')
    return data

#def _process_cfg(infile,cfg,target):
#    for section, sectiondata in cfg.items():
#        schemadata = _schema.get(section)
#        if not schemadata:
#            error.warn(f'Ignoring unknown dgbuild.cfg section "{section}".')
#            continue
#        for k,v in sectiondata.items():
#            _ = schemadata.get(k)
#            if _ is None:
#                error.warn(f'Ignoring unknown dgbuild.cfg entry name "{k}" in section "{section}".')
#                continue
#            decodefct, defval = _
#            setattr(target, f'{section}_{k}', decodefct(infile,f'{section}.{k}',v))
#    for sectionname, sectiondata in _schema.items():
#        for k,(decodefct,defval) in sectiondata.items():
#            attrname=f'{sectionname}_{k}'
#            if not hasattr(target,attrname):
#                v = None if defval is None else decodefct(infile,f'{sectionname}.{k}',defval)
#                setattr(target,attrname,v)
#

def decode_toml_textdata_to_dict( textdata : str, path = None ):
    tomllib = import_tomllib()
    descr = path or 'TOML data'
    try:
        cfg = tomllib.loads(textdata)
    except tomllib.TOMLDecodeError as e:
        error.error(f'Syntax error in {descr}: {e}')
    if not cfg:
        error.error(f'No data defined in {descr}')
    return cfg

def decode_with_schema_and_apply_result_to_obj( cfg : dict, targetobj : SingleCfg, defaultdir, srcdescr : str, ignore_build : bool ):
    schema = get_toml_schema()

    class DecodeContext:
        def __init__(self, srcdescr, default_dir):
            self.__srcdescr : str = srcdescr
            self.__item_name : str = ''
            self.__default_dir : str = default_dir
        def set_item_name( self, n ):
            self.__item_name = n
        @property
        def src_descr(self) -> str:
            return self.__srcdescr
        @property
        def default_dir(self) -> pathlib.Path | None:
            return self.__default_dir
        @property
        def item_name(self) -> str:
            assert self.__item_name
            return self.__item_name

    ctx = DecodeContext( srcdescr, defaultdir )

    #Validate+decode+apply values:
    for section, sectiondata in cfg.items():
        if ignore_build and section=='build':
            continue
        schemadata = schema.get(section)
        #forward compatibility: ignore unknown sections and keys
        if not schemadata:
            error.warn(f'Ignoring unknown dgbuild.cfg section "{section}" of {ctx.src_descr}.')
            continue
        for k,v in sectiondata.items():
            _ = schemadata.get(k)
            if _ is None:
                error.warn(f'Ignoring unknown dgbuild.cfg entry name "{k}" in section "{section}" of {ctx.src_descr}.')
                continue
            decodefct, defval = _
            ctx.set_item_name(f'{section}.{k}')
            v = decodefct(ctx,v)
            setattr( targetobj, f'{section}_{k}', v )
    #Apply default values:
    for sectionname, sectiondata in schema.items():
        if ignore_build and sectionname=='build':
            continue
        for k,(decodefct,defval) in sectiondata.items():
            attrname=f'{sectionname}_{k}'
            if not hasattr(targetobj,attrname):
                ctx.set_item_name(f'DEFAULT::{sectionname}.{k}')
                v = None if defval is None else decodefct(ctx,defval)
                setattr( targetobj,attrname, v )

    return cfg
