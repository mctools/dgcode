#ifndef dgcode_ncrystalrel_NCRNG_hh
#define dgcode_ncrystalrel_NCRNG_hh
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL
#  include "NCrystal/NCRNG.hh"
#else
#  include "NCrystalBuiltin/NCRNG.hh"
#endif
namespace NCrystalRel = NCrystal;
#endif
