#include "GriffDataRead/Track.hh"
#include "GriffDataRead/DumpObj.hh"
#include "Core/Python.hh"

namespace GriffDataRead {

  //Due to lack of overloading, we add dump methods directly to the objects in python:
  void dump_track(Track*trk) { dump(trk,false); }
  void dump_track_withpdginfo(Track*trk) { dump(trk,true); }
}

void GriffDataRead::pyexport_Track()
{
  py::class_<GriffDataRead::Track,boost::noncopyable>("Track",py::no_init)
    .def("trackID",&GriffDataRead::Track::trackID)
    .def("parentID",&GriffDataRead::Track::parentID)
    .def("nDaughters",&GriffDataRead::Track::nDaughters)
    .def("nSegments",&GriffDataRead::Track::nSegments)
    .def("weight",&GriffDataRead::Track::weight)
    .def("isPrimary",&GriffDataRead::Track::isPrimary)
    .def("isSecondary",&GriffDataRead::Track::isSecondary)
    .def("startTime",&GriffDataRead::Track::startTime)
    .def("startEKin",&GriffDataRead::Track::startEKin)
    .def("pdgCode",&GriffDataRead::Track::pdgCode)
    .def("pdgName",&GriffDataRead::Track::pdgNameCStr)
    .def("pdgType",&GriffDataRead::Track::pdgTypeCStr)
    .def("pdgSubType",&GriffDataRead::Track::pdgSubTypeCStr)
    .def("creatorProcess",&GriffDataRead::Track::creatorProcessCStr)
    .def("mass",&GriffDataRead::Track::mass)
    .def("width",&GriffDataRead::Track::width)
    .def("charge",&GriffDataRead::Track::charge)
    .def("lifeTime",&GriffDataRead::Track::lifeTime)
    .def("atomicNumber",&GriffDataRead::Track::atomicNumber)
    .def("atomicMass",&GriffDataRead::Track::atomicMass)
    .def("magneticMoment",&GriffDataRead::Track::magneticMoment)
    .def("spin",&GriffDataRead::Track::spin)
    .def("stable",&GriffDataRead::Track::stable)
    .def("shortLived",&GriffDataRead::Track::shortLived)
    .def("getDaughterID",&GriffDataRead::Track::getDaughterID)
    .def("getParent",&GriffDataRead::Track::getParent,py::return_value_policy<py::reference_existing_object>())
    .def("getDaughter",&GriffDataRead::Track::getDaughter,py::return_value_policy<py::reference_existing_object>())
    .def("getSegment",&GriffDataRead::Track::getSegment,py::return_value_policy<py::reference_existing_object>())
    .def("segmentBegin",&GriffDataRead::Track::segmentBegin,py::return_value_policy<py::reference_existing_object>())
    .def("segmentEnd",&GriffDataRead::Track::segmentEnd,py::return_value_policy<py::reference_existing_object>())
    .def("firstSegment",&GriffDataRead::Track::firstSegment,py::return_value_policy<py::reference_existing_object>())
    .def("lastSegment",&GriffDataRead::Track::lastSegment,py::return_value_policy<py::reference_existing_object>())
    .def("firstStep",&GriffDataRead::Track::firstStep,py::return_value_policy<py::reference_existing_object>())
    .def("lastStep",&GriffDataRead::Track::lastStep,py::return_value_policy<py::reference_existing_object>())
    .def("dump",&GriffDataRead::dump_track)
    .def("dump_full",&GriffDataRead::dump_track_withpdginfo)
    ;

}
