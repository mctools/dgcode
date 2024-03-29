
def main( prevent_env_setup_msg = False ):
# DGBUILD-EXPORT-ONLY>>    import sys
# DGBUILD-EXPORT-ONLY>>    if any( e.startswith('--env-') for e in sys.argv[1:] ):
# DGBUILD-EXPORT-ONLY>>        #Special short-circuit to efficiently enable standard --env-setup usage:
# DGBUILD-EXPORT-ONLY>>        if '--env-u' in sys.argv[1:]:
# DGBUILD-EXPORT-ONLY>>            #Enable --env-unsetup usage, even outside a simplebuild project:
# DGBUILD-EXPORT-ONLY>>            from .envsetup import emit_envunsetup
# DGBUILD-EXPORT-ONLY>>            emit_envunsetup()
# DGBUILD-EXPORT-ONLY>>            raise SystemExit
# DGBUILD-EXPORT-ONLY>>        if '--env-s' in sys.argv[1:]:
# DGBUILD-EXPORT-ONLY>>            #Short-circuit to efficiently enable --env-setup call:
# DGBUILD-EXPORT-ONLY>>            from .envsetup import emit_envsetup
# DGBUILD-EXPORT-ONLY>>            emit_envsetup()
# DGBUILD-EXPORT-ONLY>>            raise SystemExit
    from . import frontend
    frontend.simplebuild_main( prevent_env_setup_msg = prevent_env_setup_msg )

def unwrapped_main():
    #For the unwrapped_simplebuild entry point, presumably only called from a bash
    #function taking care of the --env-setup.
    import sys
    sys.argv[0] = 'simplebuild'
    main( prevent_env_setup_msg = True )

def sbenv_main():
    import sys
    args = sys.argv[1:]
    if not args:
        print("""Usage:

sbenv <program> [args]

Runs <program> within the simplebuild runtime environment. Note that if you wish to
make sure the codebase has been built first (with simplebuild) you should use sbrun
rather than sbenv.
""")
        sys.exit(1)
        return
    from .envsetup import create_install_env_clone
    run_env = create_install_env_clone()
    from . import utils
    import shlex
    cmd = ' '.join(shlex.quote(e) for e in args)
    utils.system(cmd,env=run_env)

def sbrun_main():
    import sys
    args = sys.argv[1:]
    if not args:
        print("""Usage:

sbrun <program> [args]

Runs simplebuild (quietly) and if it finishes successfully, then proceeds to launch
<program> within the simplebuild runtime environment.
""")
        sys.exit(1)
        return
    from .envsetup import create_install_env_clone
    run_env = create_install_env_clone()
    from . import utils
    import shlex
    cmd = ' '.join(shlex.quote(e) for e in args)
    from . import frontend
    frontend.simplebuild_main( argv = ['simplebuild',
                                       '--quiet'],
                               prevent_env_setup_msg = True )
    utils.system(cmd,env=run_env)

if __name__=='__main__':
    main()
