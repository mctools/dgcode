#include "G4Interfaces/PhysicsListPyExport.hh"
#include "G4PhysicsLists/PhysicsListEmpty.hh"

PYTHON_MODULE3
{
  PhysicsListPyExport::exportPhysList<PhysicsListEmpty>(mod);
}
