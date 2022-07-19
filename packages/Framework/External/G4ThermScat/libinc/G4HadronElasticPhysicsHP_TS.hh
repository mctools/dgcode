//This is a slightly modified version of the class G4HadronElasticPhysicsHP in
//G4.10.0.p03 to enable the thermalscattering model. Apart from the class name
//change (addinging "_TS"), lines added in the .cc file are marked with
//"ADDED". It is likely we will need to update this class again, as G4 evolves.
//
//TK 5-Feb-2014, X. X. Cai, 19-Feb-2014
//
//Update from TK March 2019: Updating to work with Geant4 v10.4.3, this involves
//  just a few lines in the .cc which changes NeutronHP with ParticleHP from
//  v10.2.0 and onwards (marked with UPDATEFORv10.2). Note that v10.5.0 introduces a much larger rewrite to the
//  G4HadronElasticPhysicsHP code. The diff was found in the G4 repo with the command:
//  git diff v10.0.0 v10.2.0 ./source/physics_lists/constructors/hadron_elastic/include/G4HadronElasticPhysicsHP.hh ./source/physics_lists/constructors/hadron_elastic/src/G4HadronElasticPhysicsHP.cc

#ifndef G4ThermScat_G4HadronElasticPhysicsHP_TS_h
#define G4ThermScat_G4HadronElasticPhysicsHP_TS_h 1

#include "globals.hh"
#include "G4VPhysicsConstructor.hh"

class G4HadronElasticPhysics;

class G4HadronElasticPhysicsHP_TS : public G4VPhysicsConstructor
{
public:

  G4HadronElasticPhysicsHP_TS(G4int ver = 1);

  virtual ~G4HadronElasticPhysicsHP_TS();

  // This method will be invoked in the Construct() method.
  // each particle type will be instantiated
  virtual void ConstructParticle();

  // This method will be invoked in the Construct() method.
  // each physics process will be instantiated and
  // registered to the process manager of each particle type
  virtual void ConstructProcess();

private:

  G4HadronElasticPhysicsHP_TS(G4HadronElasticPhysicsHP_TS &);
  G4HadronElasticPhysicsHP_TS & operator=(const G4HadronElasticPhysicsHP_TS &right);

  G4int    verbose;
  static G4ThreadLocal G4bool   wasActivated;
  static G4ThreadLocal G4HadronElasticPhysics* mainElasticBuilder;
};


#endif
