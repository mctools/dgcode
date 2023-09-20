
def main():
    import sys
    if len(sys.argv)==2 and sys.argv[1]=='--env-setup':
        #Special short-circuit to efficiently enable standard --env-setup usage:
        from .envsetup import emit_envsetup
        emit_envsetup()
        raise SystemExit
    from . import frontend # noqa: F401

def main_fix_argv0():
    import sys
    sys.argv[0] = 'dgbuild2'#FIXME: after migration put to 'dgbuild'!
    main()

if __name__=='__main__':
    main()
