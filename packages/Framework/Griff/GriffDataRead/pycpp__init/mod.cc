#include "GriffDataRead/GriffDataReader.hh"
#include "GriffDataRead/Track.hh"
#include "GriffDataRead/Segment.hh"
#include "GriffDataRead/Step.hh"
#include "GriffDataRead/DumpObj.hh"
#include "GriffDataRead/Material.hh"
#include "GriffDataRead/Element.hh"
#include "GriffDataRead/Isotope.hh"
#include "Core/Python.hh"

#include "materials.hh"

boost::shared_ptr<GriffDataReader> pyGriffDataReader_init_pylist1(py::list& l)
{
  std::vector<std::string> v;
  for (int i = 0; i < len(l); ++i)
    v.push_back(boost::python::extract<std::string>(l[i]));
  return boost::shared_ptr<GriffDataReader>(new GriffDataReader(v));
}
boost::shared_ptr<GriffDataReader> pyGriffDataReader_init_pylist2(py::list& l,unsigned n)
{
  std::vector<std::string> v;
  for (int i = 0; i < len(l); ++i)
    v.push_back(boost::python::extract<std::string>(l[i]));
  return boost::shared_ptr<GriffDataReader>(new GriffDataReader(v,n));
}

py::dict pyGriffDataReadSetup_metaData(GriffDataRead::Setup*self)
{
  py::dict d;
  const auto& m = self->metaData();
  auto itE = m.end();
  for (auto it=m.begin();it!=itE;++it)
    d[it->first]=it->second;
  return d;
}

py::dict pyGriffDataReadSetup_userData(GriffDataRead::Setup*self)
{
  py::dict d;
  const auto& m = self->userData();
  auto itE = m.end();
  for (auto it=m.begin();it!=itE;++it)
    d[it->first]=it->second;
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

PYTHON_MODULE
{
  py::import("Utils.DummyParamHolder");
  py::import("Utils.RefCountBase");

  GriffDataRead::pyexport_Step();
  GriffDataRead::pyexport_Segment();
  GriffDataRead::pyexport_Track();
  GriffDataRead::pyexport_Material();

  py::class_<GriffDataRead::GeoParams,boost::noncopyable,py::bases<Utils::DummyParamHolder> >("GeoParams",py::no_init)
    .def("getName",&GriffDataRead::GeoParams::getNameCStr)
    .def("dump",&GriffDataRead::GeoParams::dump)
    .def("dump",&pyGriffDataRead_GeoParams_dump_0args)
    ;

  py::class_<GriffDataRead::GenParams,boost::noncopyable,py::bases<Utils::DummyParamHolder> >("GenParams",py::no_init)
    .def("getName",&GriffDataRead::GenParams::getNameCStr)
    .def("dump",&GriffDataRead::GenParams::dump)
    .def("dump",&pyGriffDataRead_GenParams_dump_0args)
    ;

  py::class_<GriffDataRead::FilterParams,boost::noncopyable,py::bases<Utils::DummyParamHolder> >("FilterParams",py::no_init)
    .def("getName",&GriffDataRead::FilterParams::getNameCStr)
    .def("dump",&GriffDataRead::FilterParams::dump)
    .def("dump",&pyGriffDataRead_FilterParams_dump_0args)
    ;

  py::class_<GriffDataRead::Setup,boost::noncopyable,py::bases<Utils::RefCountBase> >("Setup",py::no_init)
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

  py::class_<GriffDataReader,boost::noncopyable>("GriffDataReader",py::init<std::string>())
    .def("__init__", py::make_constructor(&pyGriffDataReader_init_pylist1))
    .def("__init__", py::make_constructor(&pyGriffDataReader_init_pylist2))
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
    .def("setOpenMsg",&GriffDataReader::setOpenMsg).staticmethod("setOpenMsg")
    .def("openMsg",&GriffDataReader::openMsg).staticmethod("openMsg")
    .def("seekEventByIndexInCurrentFile",&GriffDataReader::seekEventByIndexInCurrentFile)
    .def("eventIndexInCurrentFile",&GriffDataReader::eventIndexInCurrentFile)
    .def("eventCheckSum",&GriffDataReader::eventCheckSum)
    .def("verifyEventDataIntegrity",&GriffDataReader::verifyEventDataIntegrity)
    ;

}
