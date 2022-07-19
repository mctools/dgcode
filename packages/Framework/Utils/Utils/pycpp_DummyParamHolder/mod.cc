#include "Core/Python.hh"
#include "Utils/DummyParamHolder.hh"

PYTHON_MODULE
{
  py::import("Utils.ParametersBase");
  py::class_<Utils::DummyParamHolder,boost::noncopyable,py::bases<Utils::ParametersBase> >("DummyParamHolder",py::init<>())
    .def(py::init<const char*>())
    ;
}
