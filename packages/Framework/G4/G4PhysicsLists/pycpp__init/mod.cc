#include "Core/Python.hh"
#include "G4PhysicsLists/PhysicsListFactory.hh"

py::list pyPhysicsListFactory_getAllReferenceListNames()
{
  py::list l;
  std::vector<std::string> v;
  PhysicsListFactory::getAllReferenceListNames(v);
  for (auto it=v.begin();it!=v.end();++it)
    l.append(*it);
  return l;
}

PYTHON_MODULE
{
  py::def("getAllReferenceListNames",pyPhysicsListFactory_getAllReferenceListNames);
}

