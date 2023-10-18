#ifndef G4Interfaces_ParticleGenPyExport_hh
#define G4Interfaces_ParticleGenPyExport_hh

#ifndef PYMODNAME
#  error "Do not include the ParticleGenPyExport.hh header for code outside pycpp_* folders."
#endif

#include "Core/Python.hh"
#include "G4Interfaces/ParticleGenBase.hh"
#include "G4Event.hh"

#include "G4VUserPrimaryGeneratorAction.hh"

namespace ParticleGenPyExport {

  template <class T>
  T* _internal_create()
  {
    return new T;
  }

#ifdef DGCODE_USEPYBIND11
  template <class T>
  py::class_<T,G4Interfaces::ParticleGenBase> exportGen(py::module_ m, const char* name)
  {
    pyextra::pyimport("G4Interfaces");
    m.def("create",&_internal_create<T>,py::return_ptr());
    return py::class_<T,G4Interfaces::ParticleGenBase>(m,name)
      .def("getAction",&T::getAction,py::return_ptr());
  }
#else
  template <class T>
  py::class_<T,boost::noncopyable,py::bases<G4Interfaces::ParticleGenBase> > exportGen(const char* name)
  {
    pyextra::pyimport("G4Interfaces");
    def("create",&_internal_create<T>,py::return_ptr());
    return py::class_<T,boost::noncopyable,py::bases<G4Interfaces::ParticleGenBase> >(name,py::no_init)
      .def("getAction",&T::getAction,py::return_ptr());
  }
#endif
}

#endif
