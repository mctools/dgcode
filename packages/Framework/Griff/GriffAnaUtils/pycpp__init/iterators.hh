namespace GriffAnaUtils {

  //NB: For a pure c++ implementation of the iterator we should wrap the
  //next() methods and inside it end the iteration by an exception:
  //
  // const TIteratedObject* obj = t->next();
  // if (!obj) {
  //   PyErr_SetString(PyExc_StopIteration, "No more data.");
  //   boost::python::throw_error_already_set();
  // }
  // return obj;
  //
  //However, that is supremely inefficient for some reason. Therefore we
  //simply return null (None) and implement the iterator on the python
  //side. This is a lot faster, go figure.

  void pyexport_iterators()
  {
    py::class_<TrackIterator,boost::noncopyable>("TrackIterator_cpp",py::init<GriffDataReader*>())
      .def("_next_cpp",&TrackIterator::next,py::return_ptr())
      .def("_reset_cpp",&TrackIterator::reset)
      .def("addFilter",&TrackIterator::addFilter,py::return_ptr());

    ITrackFilter* (SegmentIterator::*segit_addtrackfilter)(ITrackFilter*) = &SegmentIterator::addFilter;
    ISegmentFilter* (SegmentIterator::*segit_addsegmentfilter)(ISegmentFilter*) = &SegmentIterator::addFilter;

    py::class_<SegmentIterator,boost::noncopyable>("SegmentIterator_cpp",py::init<GriffDataReader*>())
      .def("_next_cpp",&SegmentIterator::next,py::return_ptr())
      .def("_reset_cpp",&SegmentIterator::reset)
      .def("addFilter",segit_addtrackfilter,py::return_ptr())
      .def("addFilter",segit_addsegmentfilter,py::return_ptr());

    ITrackFilter* (StepIterator::*stepit_addtrackfilter)(ITrackFilter*) = &StepIterator::addFilter;
    ISegmentFilter* (StepIterator::*stepit_addsegmentfilter)(ISegmentFilter*) = &StepIterator::addFilter;
    IStepFilter* (StepIterator::*stepit_addstepfilter)(IStepFilter*) = &StepIterator::addFilter;

    py::class_<StepIterator,boost::noncopyable>("StepIterator_cpp",py::init<GriffDataReader*>())
      .def("_next_cpp",&StepIterator::next,py::return_ptr())
      .def("_reset_cpp",&StepIterator::reset)
      .def("addFilter",stepit_addtrackfilter,py::return_ptr())
      .def("addFilter",stepit_addsegmentfilter,py::return_ptr())
      .def("addFilter",stepit_addstepfilter,py::return_ptr());
  }

}
