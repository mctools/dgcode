import pathlib
import os
import warnings

def _cfg_error(msg ):
    #This happens very early, before we catch error.Error exceptions, so we emit a SystemExit instead:
    #from . import error
    #error.error(msg)
    raise SystemExit('dgbuild configuration error: %s'%(msg or '<unknown error>'))

try:
    import tomllib
except ModuleNotFoundError:
    try:
        import tomli as tomllib
    except ModuleNotFoundError:
        raise SystemExit('Required toml functionality is not found. Either use '
                         'python 3.11+ or install tomli'
                         ' ("conda install -c conda-forge tomli" or "pip install tomli") ')


def _generate_schema():
    def decode_nonempty_str( infile, itemname, item ):
        if not isinstance(item,str) or not str:
            _cfg_error(f'Invalid value "{item}" for item {itemname} (expected non-empty string) in {infile}')
        return item

    def decode_nonempty_isalnum_str( infile, itemname, item ):
        if not isinstance(item,str) or not item or not item.isalnum():
            _cfg_error(f'Invalid value "{item}" for item {itemname} (expected alphanumeric string) in {infile}')
        return item

    def decode_is_list_of_nonempty_isalnum_str( infile, itemname, item ):
        if not isinstance( item, list ):
            _cfg_error('Invalid value "{item}" for item {itemname} (expected list) in {infile}')
        for i,k in enumerate(item):
            if not isinstance(k,str) or not k or not k.isalnum():
                _cfg_error('Invalid value of list entry #{i+1} in item {itemname} (expected alphanumeric string) in {infile}')
        return tuple(item)

    def decode_is_list_of_nonempty_str( infile, itemname, item ):
        if not isinstance( item, list ):
            _cfg_error(f'Invalid value "{item}" for item {itemname} (expected list) in {infile}')
        for i,k in enumerate(item):
            if not isinstance(k,str) or not k:
                _cfg_error(f'Invalid value of list entry #{i+1} in item {itemname} (expected non-empty string) in {infile}')
        return tuple(item)

    def decode_is_list_of_paths( infile, itemname, item ):
        if not isinstance( item, list ):
            _cfg_error(f'Invalid value "{item}" for item {itemname} (expected list) in {infile}')
        res=[]
        for i,k in enumerate(item):
            if not isinstance(k,str) or not k:
                _cfg_error(f'Invalid value of list entry #{i+1} in item {itemname} (expected non-empty string) in {infile}')
            p = pathlib.Path(k)
            if not p.exists():
                _cfg_error(f'Non-existing path "{p}" in list entry #{i+1} in item {itemname} in {infile}')
            res.append(p.absolute().resolve())
        return tuple(res)

    def decode_str_enum( infile, itemname, item, optionlist ):
        item = decode_nonempty_str( infile, itemname, item )
        if item not in optionlist:
            _cfg_error(f'Invalid option "{item}" for item {itemname} (must be one of {",".join(optionlist)}) in {infile}')
        return item

    def decode_nonneg_int( infile, itemname, item ):
        if not isinstance(item,int):
            _cfg_error(f'Invalid option "{item}" for item {itemname} (must be a non-negative integer) in {infile}')
        return item

    def decode_dir( infile, itemname, item ):
        item = decode_nonempty_str( infile, itemname, item )
        p = pathlib.Path(item)
        if not p.is_absolute():
            p = infile.parent / p
        return p.absolute().resolve()

    return dict( project   = dict( name        = (decode_nonempty_isalnum_str,None),
                                   topdir      = (decode_dir, '.'),
                                  ),
                 needs     = dict( bundles     = (decode_is_list_of_nonempty_isalnum_str,[]),
                                   local_paths = (decode_is_list_of_paths,[])
                                  ),
                 pkgfilter = dict( include = (decode_is_list_of_nonempty_str,[]),
                                   exclude = (decode_is_list_of_nonempty_str,[])
                                  ),
                 extdep    = dict( paths = (decode_is_list_of_paths,[]),
                                   exclude = (decode_is_list_of_nonempty_str,[]),
                                   require = (decode_is_list_of_nonempty_str,[])
                                  ),
                 build     = dict( mode = ( lambda a,b,c : decode_str_enum(a,b,c,('debug','release')), 'release' ),
                                   njobs = (decode_nonneg_int, 0),
                                   cachedir = (decode_dir, './dgbuild_cache'),
                                  ),
                )

_schema = _generate_schema()

def _locate_cfg_file():
    p = os.environ.get('DGBUILD_CFG')
    if p:
        p = pathlib.Path(p).expanduser()
        if not p.exists():
            _cfg_error('DGBUILD_CFG was set to a non-existing directory or file')
        if p.is_dir():
            p = p / 'dgbuild.cfg'
            if not p.exists():
                _cfg_error('DGBUILD_CFG was set to a directory'
                            ' with no dgbuild.cfg file in it')
        else:
            if not p.exists():
                _cfg_error('DGBUILD_CFG was set to non-existing file')
        if not p.is_absolute():
            _cfg_error('DGBUILD_CFG must be set to an absolute path')
        return p
    p = pathlib.Path('.').absolute()#NB: NOT .resolve() on purpose!
    f = p / 'dgbuild.cfg'
    if f.exists():
        return f
    for p in sorted(p.parents,reverse=True):
        f = p / 'dgbuild.cfg'
        if f.exists():
            return f

def _extract_cfg():
    f = _locate_cfg_file()
    if not f:
        _cfg_error('No dgbuild.cfg found in current or any parent directory'
                   ' (step into a directory tree with a dgbuild.cfg at the '
                   'root or set DGBUILD_CFG to a directory with a dgbuild.cfg file in it)')
    try:
        content = f.read_text()
    except UnicodeDecodeError:
        _cfg_error(f'Not a text-file: {f}')
    try:
        cfg = tomllib.loads(content)
    except tomllib.TOMLDecodeError as e:
        _cfg_error(f'Syntax error in {f}: {e}')
    if not cfg:
        _cfg_error(f'No data defined in {f}')
    return f,cfg

def _process_cfg(infile,cfg,target):
    for section, sectiondata in cfg.items():
        schemadata = _schema.get(section)
        if not schemadata:
            warnings.warn(f'Ignoring unknown dgbuild.cfg section "{section}".')
            continue
        for k,v in sectiondata.items():
            _ = schemadata.get(k)
            if _ is None:
                warnings.warn(f'Ignoring unknown dgbuild.cfg entry name "{k}" in section "{section}".')
                continue
            decodefct, defval = _
            setattr(target, f'{section}_{k}', decodefct(infile,f'{section}.{k}',v))
    for sectionname, sectiondata in _schema.items():
        for k,(decodefct,defval) in sectiondata.items():
            attrname=f'{sectionname}_{k}'
            if not hasattr(target,attrname):
                v = None if defval is None else decodefct(infile,f'{sectionname}.{k}',defval)
                setattr(target,attrname,v)

class Cfg:
    def __init__(self):
        infile,cfgdict = _extract_cfg()
        _process_cfg(infile,cfgdict,self)
        self._lock_cfg = True
    def __setattr__(self,name,value):
        if hasattr(self,'_lock_cfg'):
            raise RuntimeError("Cfg object is locked.")
        object.__setattr__(self, name, value)
    def dump(self,printfct = None):
        for ks,vs in _schema.items():
            for k in vs.keys():
                v=getattr(self,'%s_%s'%(ks,k),None)
                print('  %s.%s = %s'%(ks,k,repr(v)) )

cfg = Cfg()

if __name__=='__main__':
    print()
    cfg.dump()
