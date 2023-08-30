#ifndef dgcode_ncrystalrel_NCMatCfg_hh
#define dgcode_ncrystalrel_NCMatCfg_hh
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL
#  include "NCrystal/NCMatCfg.hh"
#else
#  include "NCrystalBuiltin/NCMatCfg.hh"
#endif
namespace NCrystalRel = NCrystal;
#endif
