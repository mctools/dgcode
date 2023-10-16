#ifndef G4Interfaces_GeoConstructPyExport_hh
#define G4Interfaces_GeoConstructPyExport_hh

#include "Core/Python.hh"
#include "G4Interfaces/GeoConstructBase.hh"
#include "G4VPhysicalVolume.hh"

namespace GeoConstructPyExport {

  template <class T>
  T* _internal_create()
  {
    return new T;
  }

  template <class T>
#ifdef DGCODE_USEPYBIND11
  py::class_<T,G4Interfaces::GeoConstructBase> exportGeo(py::module_ themod, const char* name)
#else
  py::class_<T,boost::noncopyable,py::bases<G4Interfaces::GeoConstructBase> > exportGeo(const char* name)
#endif
  {
    pyextra::pyimport("G4Interfaces");
    PYDEF2("create",&_internal_create<T>,py::return_ptr());
#ifdef DGCODE_USEPYBIND11
    return py::class_<T,G4Interfaces::GeoConstructBase>(themod,name)
      .def("Construct",&T::Construct,py::return_ptr());
#else
    return py::class_<T,boost::noncopyable,py::bases<G4Interfaces::GeoConstructBase> >(name,py::no_init)
      .def("Construct",&T::Construct,py::return_ptr());
#endif
  }


}

#endif
