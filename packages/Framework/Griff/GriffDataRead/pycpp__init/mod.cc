#include "Core/Python.hh"

#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/Track.hh"
#include "GriffDataRead/Segment.hh"
#include "GriffDataRead/Step.hh"
#include "GriffDataRead/DumpObj.hh"
#include "GriffDataRead/Material.hh"
#include "GriffDataRead/Element.hh"
#include "GriffDataRead/Isotope.hh"

namespace {
  template < typename T>
  struct BlankDeleter
  {
    void operator()(T *) const {}
  };
}

#include "step.hh"
#include "segment.hh"
#include "track.hh"
#include "materials.hh"

// #ifndef DGCODE_USEPYBIND11
// boost::shared_ptr<GriffDataReader> pyGriffDataReader_init_pylist1(py::list& l)
// {
//   std::vector<std::string> v;
//   for (int i = 0; i < len(l); ++i)
//     v.push_back(boost::python::extract<std::string>(l[i]));
//   return boost::shared_ptr<GriffDataReader>(new GriffDataReader(v));
// }
// boost::shared_ptr<GriffDataReader> pyGriffDataReader_init_pylist2(py::list& l,unsigned n)
// {
//   std::vector<std::string> v;
//   for (int i = 0; i < len(l); ++i)
//     v.push_back(boost::python::extract<std::string>(l[i]));
//   return boost::shared_ptr<GriffDataReader>(new GriffDataReader(v,n));
// }
// #endif

namespace {
  py::dict pyGriffDataReadSetup_metaData(GriffDataRead::Setup*self)
  {
    py::dict d;
    const auto& m = self->metaData();
    auto itE = m.end();
    for (auto it=m.begin();it!=itE;++it) {
#ifdef DGCODE_USEPYBIND11
      d[it->first.c_str()]=it->second.c_str();
#else
      d[it->first]=it->second;
#endif
    }
    return d;
  }

  py::dict pyGriffDataReadSetup_userData(GriffDataRead::Setup*self)
  {
    py::dict d;
    const auto& m = self->userData();
    auto itE = m.end();
    for (auto it=m.begin();it!=itE;++it)
#ifdef DGCODE_USEPYBIND11
      d[it->first.c_str()]=it->second.c_str();
#else
    d[it->first]=it->second;
#endif
    return d;
  }

  py::list pyGriffDataReadSetup_cmds(GriffDataRead::Setup*self)
  {
    py::list l;
    const auto& cmds = self->cmds();
    auto itE = cmds.end();
    for (auto it=cmds.begin();it!=itE;++it)
      l.append(*it);
    return l;
  }

  void pyGriffDataReadSetup_dump_0args(GriffDataRead::Setup*self) { self->dump(); }
  void pyGriffDataRead_GeoParams_dump_0args(GriffDataRead::GeoParams*self) { self->dump(); }
  void pyGriffDataRead_GenParams_dump_0args(GriffDataRead::GenParams*self) { self->dump(); }
  void pyGriffDataRead_FilterParams_dump_0args(GriffDataRead::FilterParams*self) { self->dump(); }
}

PYTHON_MODULE
{
  pyextra::pyimport("Utils.DummyParamHolder");
  pyextra::pyimport("Utils.RefCountBase");

#ifdef DGCODE_USEPYBIND11
  GDR_step::pyexport(m);
  GDR_segment::pyexport(m);
  GDR_track::pyexport(m);
  GDR_material::pyexport(m);
#else
  // GriffDataRead::pyexport_Step();
  // GriffDataRead::pyexport_Segment();
  // GriffDataRead::pyexport_Track();
  //  GriffDataRead::pyexport_Material();
#endif
#ifdef DGCODE_USEPYBIND11
  py::class_<GriffDataRead::GeoParams,
             std::unique_ptr<GriffDataRead::GeoParams, BlankDeleter<GriffDataRead::GeoParams>>,
             Utils::DummyParamHolder>(m,"GeoParams")
#else
  py::class_<GriffDataRead::GeoParams,boost::noncopyable,py::bases<Utils::DummyParamHolder> >("GeoParams",py::no_init)
#endif
    .def("getName",&GriffDataRead::GeoParams::getNameCStr)
    .def("dump",&GriffDataRead::GeoParams::dump)
    .def("dump",&pyGriffDataRead_GeoParams_dump_0args)
    ;

#ifdef DGCODE_USEPYBIND11
  py::class_<GriffDataRead::GenParams,
             std::unique_ptr<GriffDataRead::GenParams, BlankDeleter<GriffDataRead::GenParams>>,
             Utils::DummyParamHolder>(m,"GenParams")
#else
  py::class_<GriffDataRead::GenParams,boost::noncopyable,py::bases<Utils::DummyParamHolder> >("GenParams",py::no_init)
#endif
    .def("getName",&GriffDataRead::GenParams::getNameCStr)
    .def("dump",&GriffDataRead::GenParams::dump)
    .def("dump",&pyGriffDataRead_GenParams_dump_0args)
    ;

#ifdef DGCODE_USEPYBIND11
  py::class_<GriffDataRead::FilterParams,
             std::unique_ptr<GriffDataRead::FilterParams, BlankDeleter<GriffDataRead::FilterParams>>,
             Utils::DummyParamHolder>(m,"FilterParams")
#else
  py::class_<GriffDataRead::FilterParams,boost::noncopyable,py::bases<Utils::DummyParamHolder> >("FilterParams",py::no_init)
#endif
    .def("getName",&GriffDataRead::FilterParams::getNameCStr)
    .def("dump",&GriffDataRead::FilterParams::dump)
    .def("dump",&pyGriffDataRead_FilterParams_dump_0args)
    ;

#ifdef DGCODE_USEPYBIND11
  py::class_<GriffDataRead::Setup,
             std::unique_ptr<GriffDataRead::Setup, BlankDeleter<GriffDataRead::Setup>>,
             Utils::RefCountBase>(m,"Setup")
#else
  py::class_<GriffDataRead::Setup,boost::noncopyable,py::bases<Utils::RefCountBase> >("Setup",py::no_init)
#endif
    .def("metaData",&pyGriffDataReadSetup_metaData)
    .def("userData",&pyGriffDataReadSetup_userData)
    .def("geo",&GriffDataRead::Setup::geo,py::return_ptr())
    .def("gen",&GriffDataRead::Setup::gen,py::return_ptr())
    .def("hasFilter",&GriffDataRead::Setup::hasFilter)
    //    .def("filter",&GriffDataRead::Setup::filter,py::return_ptr())
    .def("getFilter",&GriffDataRead::Setup::filter,py::return_ptr())//Different name on py-side to avoid clash with inbuilt
    .def("cmds",&pyGriffDataReadSetup_cmds)
    .def("dump",&GriffDataRead::Setup::dump)
    .def("dump",&pyGriffDataReadSetup_dump_0args)
    ;

#ifdef DGCODE_USEPYBIND11
  py::class_<GriffDataReader,std::shared_ptr<GriffDataReader>>(m,"GriffDataReader")
    .def(py::init<std::string>())
    .def(py::init( []( py::list l )
    {
      const std::size_t n = static_cast<std::size_t>(py::len(l));
      std::vector<std::string> v;
      v.reserve(n);
      for (std::size_t i = 0; i < n; ++i)
        v.push_back( l[i].cast<std::string>() );
      return std::make_shared<GriffDataReader>(v);
    }))
    .def(py::init( []( py::list l, unsigned narg )
    {
      const std::size_t n = static_cast<std::size_t>(py::len(l));
      std::vector<std::string> v;
      v.reserve(n);
      for (std::size_t i = 0; i < n; ++i)
        v.push_back( l[i].cast<std::string>() );
      return std::make_shared<GriffDataReader>(v,narg);
    }))
#else
  // py::class_<GriffDataReader,boost::noncopyable>("GriffDataReader",py::init<std::string>())
  //   .def("__init__", py::make_constructor(&pyGriffDataReader_init_pylist1))
  //   .def("__init__", py::make_constructor(&pyGriffDataReader_init_pylist2))
#endif
    .def(py::init<std::string, unsigned>())
    .def(py::init<std::vector<std::string> >())
    .def(py::init<std::vector<std::string>, unsigned>())
    .def("hasTrackID",&GriffDataReader::hasTrackID)//
    .def("goToNextFile",&GriffDataReader::goToNextFile)
    .def("goToNextEvent",&GriffDataReader::goToNextEvent)
    .def("goToFirstEvent",&GriffDataReader::goToFirstEvent)
    .def("skipEvents",&GriffDataReader::skipEvents)//
    .def("eventActive",&GriffDataReader::eventActive)
    .def("runNumber",&GriffDataReader::runNumber)
    .def("eventNumber",&GriffDataReader::eventNumber)
    .def("currentEventVersion",&GriffDataReader::currentEventVersion)
    .def("nTracks",&GriffDataReader::nTracks)
    .def("nPrimaryTracks",&GriffDataReader::nPrimaryTracks)
    .def("loopEvents",&GriffDataReader::loopEvents)
    .def("eventStorageMode",&GriffDataReader::eventStorageModeStr)
    .def("seed",&GriffDataReader::seed)
    .def("seedStr",&GriffDataReader::seedStr)
    .def("getTrack",&GriffDataReader::getTrack,py::return_ptr())
    .def("getTrackByID",&GriffDataReader::getTrackByID,py::return_ptr())
    .def("setupChanged",&GriffDataReader::setupChanged)
    .def("setup",&GriffDataReader::setup,py::return_ptr())
    .def("allowSetupChange",&GriffDataReader::allowSetupChange)
    .def("loopCount",&GriffDataReader::loopCount)
#ifdef DGCODE_USEPYBIND11
    .def_static("setOpenMsg",&GriffDataReader::setOpenMsg)
    .def_static("openMsg",&GriffDataReader::openMsg)
#else
    .def("setOpenMsg",&GriffDataReader::setOpenMsg).staticmethod("setOpenMsg")
    .def("openMsg",&GriffDataReader::openMsg).staticmethod("openMsg")
#endif
    .def("seekEventByIndexInCurrentFile",&GriffDataReader::seekEventByIndexInCurrentFile)
    .def("eventIndexInCurrentFile",&GriffDataReader::eventIndexInCurrentFile)
    .def("eventCheckSum",&GriffDataReader::eventCheckSum)
    .def("verifyEventDataIntegrity",&GriffDataReader::verifyEventDataIntegrity)
    ;

}
