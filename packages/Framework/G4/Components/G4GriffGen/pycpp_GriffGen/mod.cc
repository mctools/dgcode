#include "G4Interfaces/ParticleGenPyExport.hh"
#include "GriffGen.hh"

PYTHON_MODULE3
{
  ParticleGenPyExport::exportGen<GriffGen>(mod, "GriffGen");
}
