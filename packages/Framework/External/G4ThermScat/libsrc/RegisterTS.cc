#include "G4ThermScat/RegisterTS.hh"
#include "G4ThermScat/G4HadronElasticPhysicsHP_TS.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include <stdexcept>
#include <cassert>

namespace G4ThermScat {

  void registerThermScatPhysics(G4VModularPhysicsList* physlist) {
    assert(physlist);
    G4int verbosity = physlist->GetVerboseLevel();
    //In a perfect world we could simply do:
    // physlist->ReplacePhysics(new G4HadronElasticPhysicsHP_TS(verbosity));
    //But the physics type fields appear to be 0 everywhere... (in G4 v 10.0.p03 - to be rechecked!)
    G4int index = 0;
    const G4VPhysicsConstructor* physconst = 0;
    bool removed=false;
    while ((physconst = physlist->GetPhysics(index++))) {
      if (dynamic_cast<const G4HadronElasticPhysicsHP*>(physconst)) {
        if (removed)
          throw std::runtime_error("TS_Wrapper Error: Did not find exactly one G4HadronElasticPhysicsHP"
                                   " on the base physics list (is it really a HP list?)");
        physlist->RemovePhysics(const_cast<G4VPhysicsConstructor*>(physconst));
        removed = true;
      }
    }
    physlist->RegisterPhysics(new G4HadronElasticPhysicsHP_TS(verbosity));

  }
}

