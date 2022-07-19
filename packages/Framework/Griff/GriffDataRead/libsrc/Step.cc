#include "GriffDataRead/Step.hh"
#include "GriffDataRead/DumpObj.hh"
#include "Core/Python.hh"

const std::string& GriffDataRead::Step::stepStatusStr() const
{
  static std::string status_strings[8];
  static bool needs_init = true;
  if (needs_init) {
    needs_init = false;
    status_strings[0] = "WorldBoundary";
    status_strings[1] = "GeomBoundary";
    status_strings[2] = "AtRestDoItProc";
    status_strings[3] = "AlongStepDoItProc";
    status_strings[4] = "PostStepDoItProc";
    status_strings[5] = "UserDefinedLimit";
    status_strings[6] = "ExclusivelyForcedProc";
    status_strings[7] = "Undefined";
  }
  unsigned s(stepStatus_raw());
  assert(s<8);
  return status_strings[s];
}

namespace GriffDataRead {
  //Due to lack of overloading, we add dump methods directly to the objects in python:
  void dump_step(Step*s) { dump(s); }

  py::tuple step_preMomentum(Step*s) { return py::make_tuple(s->preMomentumX(),s->preMomentumY(),s->preMomentumZ()); }
  py::tuple step_preLocal(Step*s) { return py::make_tuple(s->preLocalX(),s->preLocalY(),s->preLocalZ()); }
  py::tuple step_preGlobal(Step*s) { return py::make_tuple(s->preGlobalX(),s->preGlobalY(),s->preGlobalZ()); }
  py::tuple step_postMomentum(Step*s) { return py::make_tuple(s->postMomentumX(),s->postMomentumY(),s->postMomentumZ()); }
  py::tuple step_postLocal(Step*s) { return py::make_tuple(s->postLocalX(),s->postLocalY(),s->postLocalZ()); }
  py::tuple step_postGlobal(Step*s) { return py::make_tuple(s->postGlobalX(),s->postGlobalY(),s->postGlobalZ()); }


}

void GriffDataRead::pyexport_Step()
{
  py::class_<Step,boost::noncopyable>("Step",py::no_init)
    .def("getTrack",&Step::getTrack,py::return_value_policy<py::reference_existing_object>())
    .def("getSegment",&Step::getSegment,py::return_value_policy<py::reference_existing_object>())
    .def("iStep",&Step::iStep)
    .def("getNextStep",&Step::getNextStep,py::return_value_policy<py::reference_existing_object>())
    .def("getPreviousStep",&Step::getPreviousStep,py::return_value_policy<py::reference_existing_object>())
    .def("eDep",&Step::eDep)
    .def("eDepNonIonising",&Step::eDepNonIonising)
    .def("preGlobalX",&Step::preGlobalX)
    .def("preGlobalY",&Step::preGlobalY)
    .def("preGlobalZ",&Step::preGlobalZ)
    .def("postGlobalX",&Step::postGlobalX)
    .def("postGlobalY",&Step::postGlobalY)
    .def("postGlobalZ",&Step::postGlobalZ)
    .def("preLocalX",&Step::preLocalX)
    .def("preLocalY",&Step::preLocalY)
    .def("preLocalZ",&Step::preLocalZ)
    .def("postLocalX",&Step::postLocalX)
    .def("postLocalY",&Step::postLocalY)
    .def("postLocalZ",&Step::postLocalZ)
    .def("preMomentumX",&Step::preMomentumX)
    .def("preMomentumY",&Step::preMomentumY)
    .def("preMomentumZ",&Step::preMomentumZ)
    .def("postMomentumX",&Step::postMomentumX)
    .def("postMomentumY",&Step::postMomentumY)
    .def("postMomentumZ",&Step::postMomentumZ)
    .def("preTime",&Step::preTime)
    .def("postTime",&Step::postTime)
    .def("preEKin",&Step::preEKin)
    .def("postEKin",&Step::postEKin)
    .def("preAtVolEdge",&Step::preAtVolEdge)
    .def("postAtVolEdge",&Step::postAtVolEdge)
    .def("preProcessDefinedStep",&Step::preProcessDefinedStepCStr)
    .def("postProcessDefinedStep",&Step::postProcessDefinedStepCStr)
    .def("stepLength",&Step::stepLength)
    .def("dump",&GriffDataRead::dump_step)
    .def("preMomentum",&GriffDataRead::step_preMomentum)
    .def("preLocal",&GriffDataRead::step_preLocal)
    .def("preGlobal",&GriffDataRead::step_preGlobal)
    .def("postMomentum",&GriffDataRead::step_postMomentum)
    .def("postLocal",&GriffDataRead::step_postLocal)
    .def("postGlobal",&GriffDataRead::step_postGlobal)
    ;
}
