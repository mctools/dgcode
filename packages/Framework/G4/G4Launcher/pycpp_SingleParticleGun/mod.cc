#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4Launcher/SingleParticleGun.hh"

PYTHON_MODULE
{
  ParticleGenPyExport::exportGen<G4Launcher::SingleParticleGun>("SingleParticleGun");
}
