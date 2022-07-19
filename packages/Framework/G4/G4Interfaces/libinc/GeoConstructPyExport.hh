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
  py::class_<T,boost::noncopyable,py::bases<G4Interfaces::GeoConstructBase> > exportGeo(const char* name)
  {
    py::import("G4Interfaces");
    def("create",&_internal_create<T>,py::return_ptr());
    return py::class_<T,boost::noncopyable,py::bases<G4Interfaces::GeoConstructBase> >(name,py::no_init)
      .def("Construct",&T::Construct,py::return_ptr());
  }


}

#endif
