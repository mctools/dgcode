#include "G4Interfaces/ParticleGenPyExport.hh"
#include "G4StdGenerators/SimpleGen.hh"

PYTHON_MODULE3
{
  ParticleGenPyExport::exportGen<SimpleGen>(mod, "SimpleGen");
}
