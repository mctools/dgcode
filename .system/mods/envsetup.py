
def emit_envsetup( oldenv = None ):
    if oldenv is None:
        import os
        oldenv = os.environ
    from .envcfg import var

    instdir = var.install_dir_resolved
    fpcontent=[str(instdir)]

    for pathvar, inst_subdirs in sorted( var.env_paths.items() ):
        fpcontent.append( pathvar )
    assert not any(':' in e for e in fpcontent), "colons not allowed"

    #First undo effects of any previous setup:
    env_dict = env_with_previous_pathvar_changes_undone( oldenv )

    #Now inject our new ones:
    for pathvar, inst_subdirs in sorted( var.env_paths.items() ):
        for sd in inst_subdirs:
            ed = env_dict if pathvar in env_dict else oldenv
            env_dict[pathvar] = modify_path_var( pathvar, instdir / sd, env_dict = ed )

    unset_env_list = []#empty, we are overriding all of the relevant ones instead
    env_dict['DGBUILD_CURRENT_ENV'] = ':'.join(str(e) for e in fpcontent)
    env_dict['ESS_INSTALL_PREFIX']  = str(instdir)
    env_dict['ESS_DATA_DIR']        = str(instdir/'data')
    env_dict['ESS_LIB_DIR']         = str(instdir/'lib')
    env_dict['ESS_TESTREF_DIR']     = str(instdir/'tests'/'testref')
    env_dict['ESS_INCLUDE_DIR']     = str(instdir/'include')
    emit_env_list( env_dict, unset_env_list )

def emit_envunsetup( oldenv = None ):
    if oldenv is None:
        import os
        oldenv = os.environ

    env_dict = env_with_previous_pathvar_changes_undone( oldenv )
    unset_env_list = [ e for e in ['ESS_INSTALL_PREFIX','ESS_DATA_DIR','ESS_LIB_DIR',
                                   'ESS_TESTREF_DIR','ESS_INCLUDE_DIR','DGBUILD_CURRENT_ENV']
                       if e in oldenv ]
    emit_env_list( env_dict, unset_env_list )

def env_with_previous_pathvar_changes_undone( oldenv ):
    assert oldenv is not None
    env = {}
    oldfp = oldenv.get('DGBUILD_CURRENT_ENV')
    if oldfp:
        import pathlib
        _ = oldfp.split(':')
        old_instdir, old_pathvars = pathlib.Path(_[0]), set(_[1:])
        for pathvar in old_pathvars:
            env[pathvar] = modify_path_var(pathvar,old_instdir,env_dict=oldenv,remove=True)
    return env

def emit_env_list( env_dict, unset_env_list):
    import shlex
    for k,v in sorted(env_dict.items()):
        print('export %s=%s'%(k,shlex.quote(str(v or ''))))
    for k in unset_env_list:
        print(f'{k}=')
        print(f'export {k}=')
        print(f'unset {k}')
        print(f'export {k}')
        print(f'unset {k}')

def modify_path_var(varname,apath,*,env_dict,remove=False,):
    """Create new value suitable for an environment path variable (varname can for
    instance be "PATH", "LD_LIBRARY_PATH", etc.). If remove is False, it will add
    apath to the front of the list. If remove is True, all references to apath will
    be removed. In any case, duplicate entries are removed (keeping the first entry).
    """
    import pathlib
    assert isinstance(apath,pathlib.Path)
    #if not apath:
    #    return None
    apath = str(apath)#str(apath.absolute().resolve())
    assert env_dict is not None
    others = list(e for e in env_dict.get(varname,'').split(':') if (e and e!=apath))
    return ':'.join(unique_list(others if remove else ([apath]+others)))

def unique_list(seq):
    seen = set()
    return [x for x in seq if not (x in seen or seen.add(x))]
