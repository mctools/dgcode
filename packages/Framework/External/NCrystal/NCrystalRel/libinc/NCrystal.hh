#ifndef dgcode_ncrystalrel_NCrystal_hh
#define dgcode_ncrystalrel_NCrystal_hh
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL
#  include "NCrystal/NCrystal.hh"
#else
#  include "NCrystalBuiltin/NCrystal.hh"
#endif
namespace NCrystalRel = NCrystal;
#endif
