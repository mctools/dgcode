#ifndef DGCODE_USEPYBIND11
#  include <dgboost/python/enum.hpp>
#endif

namespace GriffAnaUtils {

#ifdef DGCODE_USEPYBIND11
namespace { template < typename T> struct BlankDeleter { void operator()(T *) const {} }; }
#endif



  //NB: If the user wish to add a filter to an iterator and delete that iterator
  //before creating a new one, the user should remember to call .ref() on the
  //filter to begin with!

  TrackFilter_Descendant* pycreate_TrackFilter_Descendant() { return new TrackFilter_Descendant(); }// could ref it here to chance memleaks but get convenience

  void TrackFilter_Descendant_setAncestor_1arg(TrackFilter_Descendant*tf,const GriffDataRead::Track*trk) { tf->setAncestor(trk); }


  TrackFilter_Charged* pycreate_TrackFilter_Charged() { return new TrackFilter_Charged(); }// could ref it here to chance memleaks but get convenience
  TrackFilter_Primary* pycreate_TrackFilter_Primary() { return new TrackFilter_Primary(); }// could ref it here to chance memleaks but get convenience

  TrackFilter_PDGCode* pycreate_TrackFilter_PDGCode() { return new TrackFilter_PDGCode(); }// could ref it here to chance memleaks but get convenience
  TrackFilter_PDGCode* pycreate_TrackFilter_PDGCode(int32_t c1) { return new TrackFilter_PDGCode(c1); }
  TrackFilter_PDGCode* pycreate_TrackFilter_PDGCode(int32_t c1,int32_t c2) { return new TrackFilter_PDGCode(c1,c2); }
  TrackFilter_PDGCode* pycreate_TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3) { return new TrackFilter_PDGCode(c1,c2,c3); }
  TrackFilter_PDGCode* pycreate_TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3,int32_t c4) { return new TrackFilter_PDGCode(c1,c2,c3,c4); }
  TrackFilter_PDGCode* pycreate_TrackFilter_PDGCode(int32_t c1,int32_t c2,int32_t c3,int32_t c4,int32_t c5) { return new TrackFilter_PDGCode(c1,c2,c3,c4,c5); }

  SegmentFilter_EnergyDeposition* pycreate_SegmentFilter_EnergyDeposition(const double&e) { return new SegmentFilter_EnergyDeposition(e); }
  StepFilter_EnergyDeposition* pycreate_StepFilter_EnergyDeposition(const double&e) { return new StepFilter_EnergyDeposition(e); }

  SegmentFilter_EKin* pycreate_SegmentFilter_EKin() { return new SegmentFilter_EKin(); }
  StepFilter_EKin* pycreate_StepFilter_EKin() { return new StepFilter_EKin(); }

  SegmentFilter_Volume* pycreate_SegmentFilter_Volume(const char*c1) { return new SegmentFilter_Volume(c1); }
  SegmentFilter_Volume* pycreate_SegmentFilter_Volume(const char*c1,const char*c2) { return new SegmentFilter_Volume(c1,c2); }
  SegmentFilter_Volume* pycreate_SegmentFilter_Volume(const char*c1,const char*c2,const char*c3) { return new SegmentFilter_Volume(c1,c2,c3); }

  //default arg in IFilter::setNegated
  void filter_setNegatedDefaultArg( IFilter*f ) { f->setNegated(); }
  void filter_setEnabledDefaultArg( IFilter*f ) { f->setEnabled(); }
  void filter_setDisabledDefaultArg( IFilter*f ) { f->setDisabled(); }

#ifdef DGCODE_USEPYBIND11
  void pyexport_filters( py::module_ themod)
#else
  void pyexport_filters()
#endif
  {
#ifdef DGCODE_USEPYBIND11
    py::class_<IFilter,std::unique_ptr<IFilter, BlankDeleter<IFilter>>>(themod,"IFilter")
#else
    py::class_<IFilter,boost::noncopyable>("IFilter",py::no_init)
#endif
      .def("refCount",&IFilter::refCount)//todo: should we have python module Utils.RefCountBase?
      .def("ref",&IFilter::ref)
      .def("unref",&IFilter::unref)
      .def("setNegated",&IFilter::setNegated)
      .def("setNegated",&filter_setNegatedDefaultArg)
      .def("toggleNegation",&IFilter::toggleNegation)
      .def("negated",&IFilter::negated)
      .def("enabled",&IFilter::enabled)
      .def("setEnabled",&IFilter::setEnabled)
      .def("setEnabled",&filter_setEnabledDefaultArg)
      .def("setDisabled",&IFilter::setDisabled)
      .def("setDisabled",&filter_setDisabledDefaultArg)
      ;


#ifdef DGCODE_USEPYBIND11
    py::class_<ITrackFilter,IFilter,std::unique_ptr<ITrackFilter, BlankDeleter<ITrackFilter>>>(themod,"ITrackFilter")
      .def("filter",&ITrackFilter::filter)
      ;

    py::class_<ISegmentFilter,IFilter,std::unique_ptr<ISegmentFilter, BlankDeleter<ISegmentFilter>>>(themod,"ISegmentFilter")
      .def("filter",&ISegmentFilter::filter)
      ;

    py::class_<IStepFilter,IFilter,std::unique_ptr<IStepFilter, BlankDeleter<IStepFilter>>>(themod,"IStepFilter")
      .def("filter",&IStepFilter::filter)
      ;
#else
    py::class_<ITrackFilter,boost::noncopyable,py::bases<IFilter>>("ITrackFilter",py::no_init)
      .def("filter",&ITrackFilter::filter)
      ;

    py::class_<ISegmentFilter,boost::noncopyable,py::bases<IFilter>>("ISegmentFilter",py::no_init)
      .def("filter",&ISegmentFilter::filter)
      ;

    py::class_<IStepFilter,boost::noncopyable,py::bases<IFilter>>("IStepFilter",py::no_init)
      .def("filter",&IStepFilter::filter)
      ;
#endif
    TrackFilter_PDGCode* (*ptrCreatePDGCode0)() = &pycreate_TrackFilter_PDGCode;
    TrackFilter_PDGCode* (*ptrCreatePDGCode1)(int32_t) = &pycreate_TrackFilter_PDGCode;
    TrackFilter_PDGCode* (*ptrCreatePDGCode2)(int32_t,int32_t) = &pycreate_TrackFilter_PDGCode;
    TrackFilter_PDGCode* (*ptrCreatePDGCode3)(int32_t,int32_t,int32_t) = &pycreate_TrackFilter_PDGCode;
    TrackFilter_PDGCode* (*ptrCreatePDGCode4)(int32_t,int32_t,int32_t,int32_t) = &pycreate_TrackFilter_PDGCode;
    TrackFilter_PDGCode* (*ptrCreatePDGCode5)(int32_t,int32_t,int32_t,int32_t,int32_t) = &pycreate_TrackFilter_PDGCode;

    TrackFilter_PDGCode* (TrackFilter_PDGCode::*ptrAddCodes2)(int32_t,int32_t) = &TrackFilter_PDGCode::addCodes;
    TrackFilter_PDGCode* (TrackFilter_PDGCode::*ptrAddCodes3)(int32_t,int32_t,int32_t) = &TrackFilter_PDGCode::addCodes;
    TrackFilter_PDGCode* (TrackFilter_PDGCode::*ptrAddCodes4)(int32_t,int32_t,int32_t,int32_t) = &TrackFilter_PDGCode::addCodes;
    TrackFilter_PDGCode* (TrackFilter_PDGCode::*ptrAddCodes5)(int32_t,int32_t,int32_t,int32_t,int32_t) = &TrackFilter_PDGCode::addCodes;

#ifdef DGCODE_USEPYBIND11
    py::class_<TrackFilter_PDGCode,ITrackFilter,std::unique_ptr<TrackFilter_PDGCode, BlankDeleter<TrackFilter_PDGCode>>>(themod,"TrackFilter_PDGCode")
#else
    py::class_<TrackFilter_PDGCode,boost::noncopyable,py::bases<ITrackFilter> >("TrackFilter_PDGCode",py::no_init)
#endif
#ifdef DGCODE_USEPYBIND11
      .def_static("create",ptrCreatePDGCode0,py::return_ptr())
      .def_static("create",ptrCreatePDGCode1,py::return_ptr())
      .def_static("create",ptrCreatePDGCode2,py::return_ptr())
      .def_static("create",ptrCreatePDGCode3,py::return_ptr())
      .def_static("create",ptrCreatePDGCode4,py::return_ptr())
      .def_static("create",ptrCreatePDGCode5,py::return_ptr())
#else
      .def("create",ptrCreatePDGCode0,py::return_ptr())
      .def("create",ptrCreatePDGCode1,py::return_ptr())
      .def("create",ptrCreatePDGCode2,py::return_ptr())
      .def("create",ptrCreatePDGCode3,py::return_ptr())
      .def("create",ptrCreatePDGCode4,py::return_ptr())
      .def("create",ptrCreatePDGCode5,py::return_ptr())
      .staticmethod("create")
#endif
      .def("addCodes",&TrackFilter_PDGCode::addCode,py::return_ptr())
      .def("addCodes",ptrAddCodes2,py::return_ptr())
      .def("addCodes",ptrAddCodes3,py::return_ptr())
      .def("addCodes",ptrAddCodes4,py::return_ptr())
      .def("addCodes",ptrAddCodes5,py::return_ptr())
      .def("setUnsigned",&TrackFilter_PDGCode::setUnsigned,py::return_ptr())
      .def("setSigned",&TrackFilter_PDGCode::setSigned,py::return_ptr())
      .def("isUnsigned",&TrackFilter_PDGCode::isUnsigned)
      .def("isSigned",&TrackFilter_PDGCode::isSigned)
      ;

#ifdef DGCODE_USEPYBIND11
    py::class_<TrackFilter_Descendant,ITrackFilter,std::unique_ptr<TrackFilter_Descendant, BlankDeleter<TrackFilter_Descendant>>>(themod,"TrackFilter_Descendant")
      .def_static("create",&pycreate_TrackFilter_Descendant,py::return_ptr())
#else
    py::class_<TrackFilter_Descendant,boost::noncopyable,py::bases<ITrackFilter> >("TrackFilter_Descendant",py::no_init)
      .def("create",&pycreate_TrackFilter_Descendant,py::return_ptr())
      .staticmethod("create")
#endif
      .def("setAncestor",&TrackFilter_Descendant::setAncestor)
      .def("setAncestor",&TrackFilter_Descendant_setAncestor_1arg)
      ;

#ifdef DGCODE_USEPYBIND11
    py::class_<TrackFilter_Primary,ITrackFilter,std::unique_ptr<TrackFilter_Primary, BlankDeleter<TrackFilter_Primary>>>(themod,"TrackFilter_Primary")
      .def_static("create",&pycreate_TrackFilter_Primary,py::return_ptr())
#else
    py::class_<TrackFilter_Primary,boost::noncopyable,py::bases<ITrackFilter> >("TrackFilter_Primary",py::no_init)
      .def("create",&pycreate_TrackFilter_Primary,py::return_ptr())
      .staticmethod("create")
#endif
      ;

#ifdef DGCODE_USEPYBIND11
    py::class_<TrackFilter_Charged,ITrackFilter,std::unique_ptr<TrackFilter_Charged, BlankDeleter<TrackFilter_Charged>>>(themod,"TrackFilter_Charged")
      .def_static("create",&pycreate_TrackFilter_Charged,py::return_ptr())
#else
    py::class_<TrackFilter_Charged,boost::noncopyable,py::bases<ITrackFilter> >("TrackFilter_Charged",py::no_init)
      .def("create",&pycreate_TrackFilter_Charged,py::return_ptr())
      .staticmethod("create")
#endif
      .def("setNegativeOnly",&TrackFilter_Charged::setNegativeOnly,py::return_ptr())
      .def("setPositiveOnly",&TrackFilter_Charged::setPositiveOnly,py::return_ptr())
      ;

    SegmentFilter_Volume* (*ptrCreateSegVolume1)(const char*) = &pycreate_SegmentFilter_Volume;
    SegmentFilter_Volume* (*ptrCreateSegVolume2)(const char*,const char*) = &pycreate_SegmentFilter_Volume;
    SegmentFilter_Volume* (*ptrCreateSegVolume3)(const char*,const char*,const char*) = &pycreate_SegmentFilter_Volume;

#ifdef DGCODE_USEPYBIND11
    py::class_<SegmentFilter_Volume,ISegmentFilter,std::unique_ptr<SegmentFilter_Volume, BlankDeleter<SegmentFilter_Volume>>>(themod,"SegmentFilter_Volume")
      .def_static("create",ptrCreateSegVolume1,py::return_ptr())
      .def_static("create",ptrCreateSegVolume2,py::return_ptr())
      .def_static("create",ptrCreateSegVolume3,py::return_ptr())
#else
    py::class_<SegmentFilter_Volume,boost::noncopyable,py::bases<ISegmentFilter> >("SegmentFilter_Volume",py::no_init)
      .def("create",ptrCreateSegVolume1,py::return_ptr())
      .def("create",ptrCreateSegVolume2,py::return_ptr())
      .def("create",ptrCreateSegVolume3,py::return_ptr())
      .staticmethod("create")
#endif
      .def("setPhysical",&SegmentFilter_Volume::setPhysical,py::return_ptr())
      .def("setLogical",&SegmentFilter_Volume::setLogical,py::return_ptr())
      .def("isPhysical",&SegmentFilter_Volume::isPhysical)
      .def("isLogical",&SegmentFilter_Volume::isLogical)
      ;

#ifdef DGCODE_USEPYBIND11
    py::class_<SegmentFilter_EnergyDeposition,ISegmentFilter,std::unique_ptr<SegmentFilter_EnergyDeposition, BlankDeleter<SegmentFilter_EnergyDeposition>>>(themod,"SegmentFilter_EnergyDeposition")
      .def_static("create", &pycreate_SegmentFilter_EnergyDeposition,py::return_ptr())
      ;

    py::class_<StepFilter_EnergyDeposition,IStepFilter,std::unique_ptr<StepFilter_EnergyDeposition, BlankDeleter<StepFilter_EnergyDeposition>>>(themod,"StepFilter_EnergyDeposition")
      .def_static("create", &pycreate_StepFilter_EnergyDeposition,py::return_ptr())
      ;

    py::class_<SegmentFilter_EKin,ISegmentFilter,std::unique_ptr<SegmentFilter_EKin, BlankDeleter<SegmentFilter_EKin>>>(themod,"SegmentFilter_EKin")
      .def_static("create", &pycreate_SegmentFilter_EKin,py::return_ptr())
      ;

    py::class_<StepFilter_EKin,IStepFilter,std::unique_ptr<StepFilter_EKin, BlankDeleter<StepFilter_EKin>>>(themod,"StepFilter_EKin")
      .def_static("create", &pycreate_StepFilter_EKin,py::return_ptr())
      ;
#else
    py::class_<SegmentFilter_EnergyDeposition,boost::noncopyable,py::bases<ISegmentFilter> >("SegmentFilter_EnergyDeposition",py::no_init)
      .def("create", &pycreate_SegmentFilter_EnergyDeposition,py::return_ptr())
      .staticmethod("create")
      ;

    py::class_<StepFilter_EnergyDeposition,boost::noncopyable,py::bases<IStepFilter> >("StepFilter_EnergyDeposition",py::no_init)
      .def("create", &pycreate_StepFilter_EnergyDeposition,py::return_ptr())
      .staticmethod("create")
      ;

    py::class_<SegmentFilter_EKin,boost::noncopyable,py::bases<ISegmentFilter> >("SegmentFilter_EKin",py::no_init)
      .def("create", &pycreate_SegmentFilter_EKin,py::return_ptr())
      .staticmethod("create")
      ;

    py::class_<StepFilter_EKin,boost::noncopyable,py::bases<IStepFilter> >("StepFilter_EKin",py::no_init)
      .def("create", &pycreate_StepFilter_EKin,py::return_ptr())
      .staticmethod("create")
      ;
#endif

  }
}
