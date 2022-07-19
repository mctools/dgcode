#include "Core/Python.hh"

namespace Core {
  bool is_debug_build()
  {
#ifndef NDEBUG
    return true;
#else
    return false;
#endif
  }
}

PYTHON_MODULE
{
  py::def("is_debug_build",&Core::is_debug_build);
}
