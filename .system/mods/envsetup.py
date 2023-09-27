def emit_envsetup( oldenv = None ):
    emit_env_dict( calculate_env_setup( oldenv ) )

def emit_env_unsetup( oldenv = None ):
    emit_env_dict( calculate_env_unsetup( oldenv ) )

def create_install_env_clone():
    from .envsetup import calculate_env_setup
    res_env = os.environ.copy()
    res_env_changes = calculate_env_setup()
    for k,v in res_env_changes.items():
        if v is None:
            if k in res_env:
                del res_env[k]
        else:
            res_env[k] = v
    return res_env

def apply_envsetup_to_dict( current_env ):
    for k,v in calculate_env_setup( current_env ).items():
        if v is None:
            if k in current_env:
                del current_env[k]
        else:
            current_env[k] = v

def calculate_env_unsetup( oldenv = None ):
    #returns dict( var -> value ), with None values meaning the variable should
    #be unset.
    if oldenv is None:
        import os
        oldenv = os.environ
    env_dict = env_with_previous_pathvar_changes_undone( oldenv )
    for e in ['ESS_INSTALL_PREFIX','ESS_DATA_DIR','ESS_LIB_DIR',
              'ESS_TESTREF_DIR','ESS_INCLUDE_DIR','DGBUILD_CURRENT_ENV']:
        if e in env_dict or e in oldenv:
            env_dict[e] = None
    return env_dict

def calculate_env_setup( oldenv = None ):
    #returns dict( var -> value ), with None values meaning the variable should
    #be unset.

    #First undo effects of any previous setup:
    if oldenv is None:
        import os
        oldenv = os.environ
    env_dict = calculate_env_unsetup( oldenv )

    #Figure out what we need in terms of installdir and env_path variables and
    #inject them with the correct values:
    from .envcfg import var
    instdir = var.install_dir_resolved
    fpcontent=[str(instdir)]
    for pathvar, inst_subdirs in sorted( var.env_paths.items() ):
        fpcontent.append( pathvar )
    assert not any(':' in e for e in fpcontent), "colons not allowed"

    #So inject our new variables:
    for pathvar, inst_subdirs in sorted( var.env_paths.items() ):
        prepend_entries = [ instdir / sd for sd in inst_subdirs ]
        ed = env_dict if pathvar in env_dict else oldenv
        env_dict[pathvar] = modify_path_var( pathvar,
                                             env_dict = ed,
                                             blockpath = instdir,
                                             prepend_entries = prepend_entries )


    #Set relevant non-path vars:
    env_dict['DGBUILD_CURRENT_ENV'] = ':'.join(str(e) for e in fpcontent)
    env_dict['ESS_INSTALL_PREFIX']  = str(instdir)
    env_dict['ESS_DATA_DIR']        = str(instdir/'data')
    env_dict['ESS_LIB_DIR']         = str(instdir/'lib')
    env_dict['ESS_TESTREF_DIR']     = str(instdir/'tests'/'testref')
    env_dict['ESS_INCLUDE_DIR']     = str(instdir/'include')
    return env_dict

def env_with_previous_pathvar_changes_undone( oldenv ):
    assert oldenv is not None
    env = {}
    oldfp = oldenv.get('DGBUILD_CURRENT_ENV')
    if oldfp:
        import pathlib
        _ = oldfp.split(':')
        old_instdir, old_pathvars = pathlib.Path(_[0]), set(_[1:])
        for pathvar in old_pathvars:
            env[pathvar] = modify_path_var(pathvar,env_dict=oldenv, blockpath=old_instdir)
    return env

def emit_env_dict( env_dict):
    import shlex
    for k,v in sorted(env_dict.items()):
        if v is None:
            print(f'export {k}=')#always this first, since unset statement might be an error if not already set.
            print(f'unset {k}')
        else:
            print('export %s=%s'%(k,shlex.quote(str(v))))

def modify_path_var(varname,*,env_dict, blockpath = None, prepend_entries = None):
    """Removes all references to blockpath or its subpaths from a path variable,
    prepends any requested paths, and returns the result."""
    import pathlib
    assert env_dict is not None
    assert isinstance(blockpath,pathlib.Path)
    assert blockpath.is_dir()
    if prepend_entries:
        res = prepend_entries[:]
    else:
        res = []
    for e in env_dict.get(varname,'').split(':'):
        if e and ( blockpath is None or not pathlib.Path(e).is_relative_to( blockpath ) ):
            res.append(str(e))
    return ':'.join(unique_list(str(e) for e in res))

def unique_list(seq):
    seen = set()
    return [x for x in seq if not (x in seen or seen.add(x))]
