#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4StdGenerators/ProfiledBeamGen.hh"

PYTHON_MODULE3
{
  ParticleGenPyExport::exportGen<ProfiledBeamGen>(mod, "ProfiledBeamGen");
}
