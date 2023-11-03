#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4Launcher/SingleParticleGun.hh"

PYTHON_MODULE3
{
  ParticleGenPyExport::exportGen<G4Launcher::SingleParticleGun>(mod,"SingleParticleGun");
}
