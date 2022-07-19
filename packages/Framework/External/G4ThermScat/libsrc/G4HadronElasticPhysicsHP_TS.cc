//See comment in header file.

#include "G4Version.hh"//ADDED

#include "G4ThermScat/G4HadronElasticPhysicsHP_TS.hh"
#include "G4SystemOfUnits.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4Neutron.hh"
#include "G4HadronicProcess.hh"
#include "G4HadronElastic.hh"
#if G4VERSION_NUMBER >= 1020 // UPDATEFORv10.2
#include "G4ParticleHPElastic.hh" // UPDATEFORv10.2
#include "G4ParticleHPElasticData.hh" // UPDATEFORv10.2
#else // UPDATEFORv10.2
#include "G4NeutronHPElastic.hh"
#include "G4NeutronHPElasticData.hh"
#endif // UPDATEFORv10.2
#include "G4NeutronHPThermalScattering.hh"//ADDED
#include "G4NeutronHPThermalScatteringData.hh"//ADDED

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(G4HadronElasticPhysicsHP_TS);

G4ThreadLocal G4bool G4HadronElasticPhysicsHP_TS::wasActivated = false;
G4ThreadLocal G4HadronElasticPhysics* G4HadronElasticPhysicsHP_TS::mainElasticBuilder = 0;

G4HadronElasticPhysicsHP_TS::G4HadronElasticPhysicsHP_TS(G4int ver)
  : G4VPhysicsConstructor("hElasticWEL_CHIPS_HP_TS"), verbose(ver)
{
  if(verbose > 1) {
    G4cout << "### G4HadronElasticPhysicsHP_TS: " << GetPhysicsName()
	   << G4endl;
  }
  mainElasticBuilder = new G4HadronElasticPhysics(verbose);
}

G4HadronElasticPhysicsHP_TS::~G4HadronElasticPhysicsHP_TS()
{
  delete mainElasticBuilder;
}

void G4HadronElasticPhysicsHP_TS::ConstructParticle()
{
  // G4cout << "G4HadronElasticPhysics::ConstructParticle" << G4endl;
  mainElasticBuilder->ConstructParticle();
}

void G4HadronElasticPhysicsHP_TS::ConstructProcess()
{
  if(wasActivated) { return; }
  wasActivated = true;
  //Needed because this is a TLS object and this method is called by all threads
  if ( ! mainElasticBuilder ) mainElasticBuilder = new G4HadronElasticPhysics(verbose);
  mainElasticBuilder->ConstructProcess();

  mainElasticBuilder->GetNeutronModel()->SetMinEnergy(19.5*MeV);

  G4HadronicProcess* hel = mainElasticBuilder->GetNeutronProcess();
  G4NeutronHPThermalScattering* theNeutronThermalElasticModel = new G4NeutronHPThermalScattering();//ADDED
#if G4VERSION_NUMBER >= 1020 // UPDATEFORv10.2
  G4ParticleHPElastic* hp = new G4ParticleHPElastic(); // UPDATEFORv10.2
#else // UPDATEFORv10.2
  G4NeutronHPElastic* hp = new G4NeutronHPElastic();
#endif // UPDATEFORv10.2
  hp->SetMinEnergy(4.0*eV);//ADDED
  hel->RegisterMe(hp);
  hel->RegisterMe(theNeutronThermalElasticModel);//ADDED
#if G4VERSION_NUMBER >= 1020 // UPDATEFORv10.2
  hel->AddDataSet(new G4ParticleHPElasticData()); // UPDATEFORv10.2
#else // UPDATEFORv10.2
  hel->AddDataSet(new G4NeutronHPElasticData());
#endif // UPDATEFORv10.2
#if G4VERSION_NUMBER > 1003//ADDED
  hel->AddDataSet(new G4NeutronHPThermalScatteringData());//ADDED
#else // for versions 1003 and below //ADDED
  G4NeutronHPThermalScatteringData* neutronTSData = new G4NeutronHPThermalScatteringData();//ADDED
  neutronTSData->AddUserThermalScatteringFile("TS_H_of_Para_Hydrogen","h_para_h2");//ADDED
  neutronTSData->AddUserThermalScatteringFile("TS_H_of_Ortho_Hydrogen","h_ortho_h2");//ADDED
  hel->AddDataSet(neutronTSData);//ADDED
#endif//ADDED

  if(verbose > 1) {
    G4cout << "### G4HadronElasticPhysicsHP_TS is constructed "
	   << G4endl;
  }
}
