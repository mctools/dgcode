#dgcode-aware version
import os
import pathlib
_=pathlib.Path(os.environ['ESS_LIB_DIR'])
assert _.is_dir()
_=list(_.glob('libPKG__NCrystalRel.*'))
assert len(_)==1
liblocation=_[0]

