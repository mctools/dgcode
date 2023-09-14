class PkgFilter:

    """Parse and implement the package filters defined in dgbuild.cfg data."""
    def __init__( self, filter_definition_list ):
        import fnmatch
        import re
        filters = []
        for forig in filter_definition_list:
            f = forig.strip()
            negated = f.startswith('!')
            if negated:
                f = f[1:]
            if not f:
                #Silently ignore empty entries (both "!" and "")
                continue
            regex = fnmatch.translate(f) if not f.startswith('RE::') else f[4:]
            try:
                reobj = re.compile(regex)
            except re.error as re:
                from . import error
                error.error(f'Invalid selection pkg filter pattern: "{forig}"')
            onreldir = '/' in f
            filters.append( ( negated, onreldir, reobj ) )
        self.__filters = tuple(e for e in filters)

    def apply( self, pkg_names_and_reldirs ):
        if not self.__filters:
            for name, _ in pkg_names_and_reldirs:
                yield name
            return
        for name, reldir in pkg_names_and_reldirs:
            blocked = False
            for negated, onreldir, reobj in self.__filters:
                p = './%s'%reldir if onreldir else name
                hasmatch = reobj.search( p ) is not None
                #print("         SEARCHING",reobj.pattern,"VS",p,"GIVES",hasmatch)
                if hasmatch == negated:
                    blocked = True
                    break
            if not blocked:
                yield name
