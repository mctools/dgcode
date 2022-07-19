#ifndef G4ThermScat__RegisterTS_hh
#define G4ThermScat__RegisterTS_hh

#include "G4VModularPhysicsList.hh"

namespace G4ThermScat {

  //Removes the existing G4HadronElasticPhysicsHP process and adds instead a
  //G4HadronElasticPhysicsHP_TS process:
  void registerThermScatPhysics(G4VModularPhysicsList* physlist);

}

#endif
