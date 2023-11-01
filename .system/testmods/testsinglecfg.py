from ess_dgbuild_internals.singlecfg import SingleCfg
import ess_dgbuild_internals.error as error
error.default_error_type = RuntimeError
error.fmt_dgbuild_warnings()


import pathlib

def main( verbose = True ):
    try:
        SingleCfg(_private_construct_key=None)
    except TypeError as e:
        assert 'direct initialisation disallowed' in str(e).lower()
    class FakeModule:
        def dgbuild_bundle_list( self ):
            import pathlib
            return pathlib.Path('/tmp/no/where/does/not/exist/simplebuild.cfg')
        #return 'mystuff'
        #def dgbuild_bundle_pkgroot( self ):

    SingleCfg.create_from_object_methods( FakeModule(), ignore_build = True )
    class FakeModule2:
        def dgbuild_bundle_toml_cfg( self ):
            return """
[project]
  name = "mystuff2"
  pkg_root = "./blapkgs"
"""
    SingleCfg.create_from_object_methods( FakeModule2(), default_dir = pathlib.Path('.') )
    class FakeModule3:
        @property
        def __file__( self ):
            return __file__
        def dgbuild_bundle_toml_cfg( self ):
            return """
[project]
  name = "mystuff2"
  pkg_root = "./blapkgs"
"""
    SingleCfg.create_from_object_methods( FakeModule3() )
    class FakeModule4:
        def dgbuild_default_dir( self ):
            return pathlib.Path(__file__).parent
        def dgbuild_bundle_toml_cfg( self ):
            return """
[project]
  name = "mystuff2"
  pkg_root = "./blapkgs"
"""
    SingleCfg.create_from_object_methods( FakeModule4() )
    return

if __name__ == '__main__':
    main()
