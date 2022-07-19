#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4StdGenerators/SimpleGen.hh"

PYTHON_MODULE
{
  ParticleGenPyExport::exportGen<SimpleGen>("SimpleGen");
}
