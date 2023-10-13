#include "Core/Python.hh"
#include "Utils/DummyParamHolder.hh"

PYTHON_MODULE
{
  py::pyimport("Utils.ParametersBase");
#ifdef DGCODE_USEPYBIND11
  py::class_<Utils::DummyParamHolder PYBOOSTNONCOPYABLE , Utils::ParametersBase >(PYMOD "DummyParamHolder" PYBOOSTNOINIT)
#else
  py::class_<Utils::DummyParamHolder PYBOOSTNONCOPYABLE , py::bases<Utils::ParametersBase> >(PYMOD "DummyParamHolder" PYBOOSTNOINIT)
#endif
    .def(py::init<const char*>())
    .def(py::init<>())
    ;
}
