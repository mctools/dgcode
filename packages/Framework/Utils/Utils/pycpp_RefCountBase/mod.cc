#include "Core/Python.hh"
#include "Utils/RefCountBase.hh"

PYTHON_MODULE
{
  py::class_<Utils::RefCountBase,boost::noncopyable>("RefCountBase",py::no_init)
    .def("refCount",&Utils::RefCountBase::refCount)
    .def("ref",&Utils::RefCountBase::ref)
    .def("unref",&Utils::RefCountBase::unref)
    .def("unrefNoDelete",&Utils::RefCountBase::unrefNoDelete)
    ;
}
