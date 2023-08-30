def _is_using_ncrystal_builtin():
    import os
    import pathlib
    _=os.environ.get('ESS_INSTALL_PREFIX')
    assert _ is not None
    _ = pathlib.Path(_) / 'python'
    assert _.is_dir()
    return ( _ / 'NCrystalBuiltin' / '__init__.py' ).exists()

if _is_using_ncrystal_builtin():
    from NCrystalBuiltin import *
else:
    #FIXME: skip a package called NCrystal in ESS_INSTALL_PREFIX if it exists? Or simply rename ncrystaldev/dist/NCrystal to ncrystaldev/dist/NCrystalDev?
    from NCrystal import *

