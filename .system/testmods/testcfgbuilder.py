from ess_dgbuild_internals.cfgbuilder import SingleCfg, CfgBuilder


def main( verbose = True ):
    #SingleCfg(_private_construct_key=None)
    return
#    default_pkgs = ( 'Core', 'subdir/Utils', 'subdir/subdir/CoreTests' )
#    all_pkgs = '__all__pkgs__'
#
#    def test( filters, expected_selected_pkgs, pkgs = None ):
#        pkgname = lambda reldir : reldir.split('/')[-1]
#        pkgs = set(pkgs or default_pkgs)
#        if expected_selected_pkgs==all_pkgs:
#            expected_selected_pkgs = set( pkgname(e) for e in pkgs )
#        else:
#            expected_selected_pkgs = set(expected_selected_pkgs)
#        if verbose:
#            print(f"===> test( {filters=},\n           {expected_selected_pkgs=},\n           {pkgs=}\n     )")
#        pkgfilter = PkgFilter( filters )
#        pkglist = ( ( pkgname(e), e ) for e in pkgs )
#        selected_actual = set( e for e in pkgfilter.apply( pkglist ) )
#        if verbose:
#            print("    -> selected expected packages!\n")
#
#        expected = set(pkgname(e) for e in expected_selected_pkgs)
#
#        if expected != selected_actual:
#            print("ERROR:")
#            print(f"    Expected selection of: {list(expected)}")
#            print(f"    ............. but got: {list(selected_actual)}")
#            raise SystemExit(1)
#
#    test( [], default_pkgs )
#    test( ['Core'], ['Core'] )
#    test( ['Core*'], ['Core','CoreTests'] )
#    test( ['subdir'], [] )
#    test( ['subdir/'], [] )
#    test( ['subdir/*'], ['Utils','CoreTests'] )
#    test( ['./'], [] )
#    test( ['*/*'], all_pkgs )
#    test( ['./*'], all_pkgs )
#    test( ['./*/*'], ['Utils','CoreTests'] )
#    test( ['./*/*/*'], ['CoreTests'] )
#    test( ['./*','!Core'],['Utils','CoreTests'] )
#    test( ['./*','!Core'],['Utils','CoreTests'] )
#    test( ['subdir/*','!Core*'],['Utils'] )
#    test( ['subdir/*','!*/U*'],['CoreTests'] )
#
    #negations, all, regex::



if __name__ == '__main__':
    main()
