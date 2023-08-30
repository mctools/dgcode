#ifndef dgcode_ncrystalrel_NCVersion_hh
#define dgcode_ncrystalrel_NCVersion_hh
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL
#  include "NCrystal/NCVersion.hh"
#else
#  include "NCrystalBuiltin/NCVersion.hh"
#endif
namespace NCrystalRel = NCrystal;
#endif
