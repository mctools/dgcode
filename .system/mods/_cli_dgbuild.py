
def main( prevent_env_setup_msg = False ):
# DGBUILD-EXPORT-ONLY>>    import sys
# DGBUILD-EXPORT-ONLY>>    if any( e.startswith('--env-') for e in sys.argv[1:] ):
# DGBUILD-EXPORT-ONLY>>        #Special short-circuit to efficiently enable standard --env-setup usage:
# DGBUILD-EXPORT-ONLY>>        if '--env-u' in sys.argv[1:]:
# DGBUILD-EXPORT-ONLY>>            #Enable --env-u[nsetup] usage, even outside a dgbuild project:
# DGBUILD-EXPORT-ONLY>>            from .envsetup import emit_envunsetup
# DGBUILD-EXPORT-ONLY>>            emit_envunsetup()
# DGBUILD-EXPORT-ONLY>>            raise SystemExit
# DGBUILD-EXPORT-ONLY>>        if '--env-s' in sys.argv[1:]:
# DGBUILD-EXPORT-ONLY>>            #Short-circuit to efficiently enable --env-setup call:
# DGBUILD-EXPORT-ONLY>>            from .envsetup import emit_envsetup
# DGBUILD-EXPORT-ONLY>>            emit_envsetup()
# DGBUILD-EXPORT-ONLY>>            raise SystemExit
    from . import frontend
    frontend.dgbuild_main( prevent_env_setup_msg = prevent_env_setup_msg )

def unwrapped_main():
    #For the unwrapped_dgbuild entry point, presumably only called from a bash
    #function taking care of the --env-setup.
    import sys
    sys.argv[0] = 'dgbuild2'#FIXME: after migration put to 'dgbuild'!
    main( prevent_env_setup_msg = True )

if __name__=='__main__':
    main()
