#include "Core/Python.hh"
#include "G4OSG/Viewer.hh"

namespace G4OSG {
  int run_viewer()
  {
    G4OSG::Viewer * viewer = new G4OSG::Viewer();
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }
  void decode_py_list(py::list input, std::set<uint64_t>& o)
  {
    py::ssize_t n = py::len(input);
    for(py::ssize_t i=0;i<n;++i) {
      py::object elem = input[i];
      uint64_t evt = py::extract<uint64_t>(elem);
      o.insert(evt);
    }
  }

  int run_viewer_with_griff(const char* griff_file, py::list pyevents)
  {
    std::set<uint64_t> events;
    decode_py_list(pyevents,events);
    G4OSG::Viewer * viewer = new G4OSG::Viewer(griff_file,true,events);
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }
  int run_viewer_with_griff_1arg(const char* griff_file)
  { return run_viewer_with_griff(griff_file,py::list()); }

  int run_viewer_with_griff_no_geom(const char* griff_file, py::list pyevents)
  {
    std::set<uint64_t> events;
    decode_py_list(pyevents,events);
    G4OSG::Viewer * viewer = new G4OSG::Viewer(griff_file,false,events);
    int ec = viewer->run();
    if (ec)
      return ec;
    delete viewer;
    return 0;
  }
  int run_viewer_with_griff_no_geom_1arg(const char* griff_file)
  { return run_viewer_with_griff_no_geom(griff_file,py::list()); }

}

PYTHON_MODULE
{
  py::def("run",&G4OSG::run_viewer);
  py::def("run",&G4OSG::run_viewer_with_griff);
  py::def("run",&G4OSG::run_viewer_with_griff_1arg);
  py::def("run_nogeom",&G4OSG::run_viewer_with_griff_no_geom);
  py::def("run_nogeom",&G4OSG::run_viewer_with_griff_no_geom_1arg);
}
