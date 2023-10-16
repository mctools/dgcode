//all includes here to make sure Core/Python is last (workaround for osx bug)

#include "GriffAnaUtils/TrackIterator.hh"
#include "GriffAnaUtils/SegmentIterator.hh"
#include "GriffAnaUtils/StepIterator.hh"
#include "GriffAnaUtils/IFilter.hh"
#include "GriffAnaUtils/ITrackFilter.hh"
#include "GriffAnaUtils/TrackFilter_Charged.hh"
#include "GriffAnaUtils/TrackFilter_Primary.hh"
#include "GriffAnaUtils/TrackFilter_Descendant.hh"
#include "GriffAnaUtils/TrackFilter_PDGCode.hh"
#include "GriffAnaUtils/SegmentFilter_Volume.hh"
#include "GriffAnaUtils/SegmentFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/StepFilter_EnergyDeposition.hh"
#include "GriffAnaUtils/SegmentFilter_EKin.hh"
#include "GriffAnaUtils/StepFilter_EKin.hh"
#include "Core/Python.hh"

#include "filters.hh"
#include "iterators.hh"

PYTHON_MODULE
{
  pyextra::pyimport("GriffDataRead");
#ifdef DGCODE_USEPYBIND11
  GriffAnaUtils::pyexport_filters(m);
  GriffAnaUtils::pyexport_iterators(m);
#else
  GriffAnaUtils::pyexport_filters();
  GriffAnaUtils::pyexport_iterators();
#endif
}
