from ess_dgbuild_internals.pkgfilter import PkgFilter


def main( verbose = True ):
    default_pkgs = ( 'Core', 'subdir/Utils', 'subdir/subdir/CoreTests', 'otherdir/FooUtils', 'SomePkg' )

    def test( filters, expected_selected_pkgs, pkgs = None ):
        pkgname = lambda reldir : reldir.split('/')[-1]
        pkgs = set(pkgs or default_pkgs)
        expected_selected_pkgs = set(expected_selected_pkgs)
        if verbose:
            print(f"===> test( {filters=},\n           {expected_selected_pkgs=},\n           {pkgs=}\n     )")
        pkgfilter = PkgFilter( filters )
        pkgfilter.dump()
        pkglist = ( ( pkgname(e), e ) for e in pkgs )
        selected_actual = set( e for e in pkgfilter.apply( pkglist ) )
        if verbose:
            print("    -> selected expected packages!\n")

        expected = set(pkgname(e) for e in expected_selected_pkgs)

        if expected != selected_actual:
            print("ERROR:")
            print(f"    Expected selection of: {list(expected)}")
            print(f"    ............. but got: {list(selected_actual)}")
            raise SystemExit(1)

    test( [], default_pkgs )
    test( ['Core'], ['Core'] )
    test( ['CoreTests'], ['CoreTests'] )
    test( ['Utils'], ['Utils'] )
    test( ['Utils','CoreTests'], ['Utils','CoreTests'] )
    test( ['Core*'], ['Core','CoreTests'] )
    test( ['subdir'], [] )
    test( ['subdir/'], [] )
    test( ['subdir/*'], ['Utils','CoreTests'] )
    test( ['./'], [] )
    test( ['*/*'], ['Utils','CoreTests','FooUtils'] )
    test( ['./*'], default_pkgs )
    test( ['./*/*'], ['Utils','CoreTests','FooUtils'] )
    test( ['./*/*/*'], ['CoreTests'] )
    test( ['./*','!Core'],['Utils','CoreTests','FooUtils','SomePkg'] )
    test( ['subdir/*','!Core*'],['Utils'] )
    test( ['subdir/*','!*/U*'],['CoreTests'] )
    test( ['RE::^Core.*$'],['Core'] )
    test( ['RE::Core$'],['Core'] )
    test( ['RE::Core'],['Core'] )
    test( ['RE::CoreTests'],[] )
    test( ['RE::.*Core.*'],['Core','CoreTests'] )
    test( ['!./subdir/*','*Utils','RE::.*Core.*'],['Core','FooUtils'] )
    test( ['!RE::^subdir/.*','*Utils','RE::.*Core.*'],['Core','FooUtils'] )
    test( ["Foo*","CoreTests",'!FooUtils'], ['CoreTests'])


    #negations, all, regex::

if __name__ == '__main__':
    main()
