#ifndef G4Interfaces_PhysicsListPyExport_hh
#define G4Interfaces_PhysicsListPyExport_hh

#include "Core/Python.hh"
#include <cassert>
#include "Core/String.hh"
#include "G4Interfaces/PhysListProviderBase.hh"

namespace PhysicsListPyExport {

  template <class T>
  class Provider : public G4Interfaces::PhysListProviderBase {
  public:
    Provider(const char*name) : G4Interfaces::PhysListProviderBase(name) {}
    virtual ~Provider(){}
    virtual G4VUserPhysicsList * construct() {
      return new T;
    }
  };

  template <class T>
  Provider<T>* _internal_create_provider(const char* provider_name)
  {
    return new Provider<T>(provider_name);
  }

  //Not exposing base directly like for gen/geo (could do if needed of course).
  //Also, we are skipping the name parameter => one physics list per module.
  //The reason being that we want to be able to identify available physics lists
  //merely from looking at module names.

  template <class T>
  py::class_<T,boost::noncopyable> exportPhysList()
  {
    //remember, if not using gcc, __GNUCC__ will evaluate to 0 (we want to exclude gcc <= 4.5)
#if __GNUC__ == 0 || __GNUC__ > 4 || __GNUC_MINOR__ > 5

    static_assert(strncmp("g4physlist_",BOOST_STRINGIZE(PYMODNAME),11)==0
                     && "ERROR: Names of python modules containing G4 physics lists must all be prefixed with g4physlist_");
#endif
    std::string bstr = BOOST_STRINGIZE(PYMODNAME);
    const char * class_name = &bstr[11];

    py::import("G4Interfaces");

    def("create_provider",&_internal_create_provider<T>,py::return_ptr());

    py::class_<Provider<T>,boost::noncopyable,py::bases<G4Interfaces::PhysListProviderBase> >((std::string("Provider__")+class_name).c_str(),py::no_init);

    return py::class_<T,boost::noncopyable>(class_name,py::no_init);
  }
}

#endif
