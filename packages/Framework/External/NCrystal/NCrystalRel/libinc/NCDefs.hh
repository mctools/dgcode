#ifndef dgcode_ncrystalrel_NCDefs_hh
#define dgcode_ncrystalrel_NCDefs_hh
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL
#  include "NCrystal/NCDefs.hh"
#else
#  include "NCrystalBuiltin/NCDefs.hh"
#endif
namespace NCrystalRel = NCrystal;
#endif
