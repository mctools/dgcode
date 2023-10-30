#ifndef G4Materials_NCG4Utils_hh
#define G4Materials_NCG4Utils_hh

class G4Material;
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL // DGBUILD-NO-EXPORT
#  include "NCrystal/NCMatCfg.hh" // DGBUILD-NO-EXPORT
#else // DGBUILD-NO-EXPORT
#  include "NCrystalBuiltin/NCMatCfg.hh" // DGBUILD-NO-EXPORT
#endif // DGBUILD-NO-EXPORT
// DGBUILD-EXPORT-ONLY>>#include "NCrystal/NCMatCfg.hh"
#include <vector>

namespace NCG4Utils {

  //Create multi-phase alloys (really: weighted mixtures of crystallite). Note
  //that this function always creates a new G4Material when called, and does
  //*NOT* employ any caching behind the scenes. It is the callers responsibility
  //that the provided material name is unique for each call.
  typedef std::vector<std::pair<std::string,double> > AlloyList;
#ifdef DGCODE_USE_SYSTEM_NCRYSTAL // DGBUILD-NO-EXPORT
  typedef std::vector<std::pair<NCrystal::MatCfg,double> > AlloyListMatCfg; // DGBUILD-NO-EXPORT
#else // DGBUILD-NO-EXPORT
  typedef std::vector<std::pair<NCrystalRel::MatCfg,double> > AlloyListMatCfg; // DGBUILD-NO-EXPORT
#endif // DGBUILD-NO-EXPORT
// DGBUILD-EXPORT-ONLY>>  typedef std::vector<std::pair<NCrystal::MatCfg,double> > AlloyListMatCfg;
  G4Material * createPCAlloy( const std::string& name, const AlloyList& nccfgs_and_weights, double density = 0 );
  G4Material * createPCAlloy( const std::string& name, const AlloyListMatCfg& nccfgs_and_weights, double density = 0 );

}

#endif
