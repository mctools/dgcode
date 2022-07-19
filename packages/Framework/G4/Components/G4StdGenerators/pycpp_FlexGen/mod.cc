#include "G4Interfaces/ParticleGenPyExport.hh"
#include "FlexGen.hh"

PYTHON_MODULE
{
  ParticleGenPyExport::exportGen<FlexGen>("FlexGen");
}
