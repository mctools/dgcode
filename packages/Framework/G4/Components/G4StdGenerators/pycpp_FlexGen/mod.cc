#include "G4Interfaces/ParticleGenPyExport.hh"
#include "FlexGen.hh"

PYTHON_MODULE3
{
  ParticleGenPyExport::exportGen<FlexGen>(mod, "FlexGen");
}
