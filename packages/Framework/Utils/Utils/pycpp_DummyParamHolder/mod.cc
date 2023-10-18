#include "Core/Python.hh"
#include "Utils/DummyParamHolder.hh"

PYTHON_MODULE3
{
  pyextra::pyimport("Utils.ParametersBase");
  py::class_<Utils::DummyParamHolder, Utils::ParametersBase >(mod, "DummyParamHolder")
    .def(py::init<const char*>())
    .def(py::init<>())
    ;
}
