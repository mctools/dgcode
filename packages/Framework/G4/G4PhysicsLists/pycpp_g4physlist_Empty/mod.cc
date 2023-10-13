#include "G4Interfaces/PhysicsListPyExport.hh"
#include "G4PhysicsLists/PhysicsListEmpty.hh"

PYTHON_MODULE
{
#if defined(DGCODE_USEPYBIND11)
  PhysicsListPyExport::exportPhysList<PhysicsListEmpty>(m);
#else
  PhysicsListPyExport::exportPhysList<PhysicsListEmpty>();
#endif
}
