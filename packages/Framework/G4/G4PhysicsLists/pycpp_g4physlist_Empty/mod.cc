#include "G4Interfaces/PhysicsListPyExport.hh"
#include "G4PhysicsLists/PhysicsListEmpty.hh"

PYTHON_MODULE
{
  PhysicsListPyExport::exportPhysList<PhysicsListEmpty>();
}
