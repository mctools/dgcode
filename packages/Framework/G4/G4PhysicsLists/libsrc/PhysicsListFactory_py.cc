#include "Core/Python.hh"
#include "G4PhysicsLists/PhysicsListFactory.hh"
#include "G4Interfaces/PhysListProviderBase.hh"
#include "G4VUserPhysicsList.hh"

G4VUserPhysicsList * PhysicsListFactory::attemptCreateCustomPhysicsList(const std::string& name)
{
  py::ensurePyInit();
  auto mod = py::import("G4PhysicsLists");
  auto pyp = mod.attr("extractProvider")(name);
  if (!pyp)
    return 0;
  G4Interfaces::PhysListProviderBase* p = py::extract<G4Interfaces::PhysListProviderBase*>(pyp);
  assert(p);
  return p->construct();
}

