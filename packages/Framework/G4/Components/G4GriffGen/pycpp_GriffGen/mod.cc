#include "G4Interfaces/ParticleGenPyExport.hh"
#include "GriffGen.hh"

PYTHON_MODULE
{
  ParticleGenPyExport::exportGen<GriffGen>("GriffGen");
}
