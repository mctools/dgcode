#ifndef G4Interfaces_StepFilterPyExport_hh
#define G4Interfaces_StepFilterPyExport_hh

#include "Core/Python.hh"
#include "G4Interfaces/StepFilterBase.hh"
#include "G4Step.hh"

namespace StepFilterPyExport {

  template <class T>
  T* _internal_create()
  {
    return new T;
  }

  template <class T>
  void exportFilter(const char* name)
  {
    py::import("G4Interfaces");
    def("create",&_internal_create<T>,py::return_ptr());
    py::class_<T,boost::noncopyable,py::bases<G4Interfaces::StepFilterBase> >(name,py::no_init)
      ;
  }

}

#endif
