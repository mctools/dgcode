#ifndef G4Interfaces_StepFilterPyExport_hh
#define G4Interfaces_StepFilterPyExport_hh

#ifndef PYMODNAME
#  error "Do not include the StepFilterPyExport.hh header for code outside pycpp_* folders."
#endif

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
#ifdef DGCODE_USEPYBIND11
  void exportFilter(py::module_ themod,const char* name)
#else
  void exportFilter(const char* name)
#endif
  {
    pyextra::pyimport("G4Interfaces");
    PYDEF2("create",&_internal_create<T>,py::return_ptr());
#ifdef DGCODE_USEPYBIND11
    py::class_<T,G4Interfaces::StepFilterBase>(themod,name)
#else
    py::class_<T,boost::noncopyable,py::bases<G4Interfaces::StepFilterBase> >(name,py::no_init)
#endif
      ;
  }

}

#endif
