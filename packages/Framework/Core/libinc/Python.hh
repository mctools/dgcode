#ifndef Core_Python_hh
#define Core_Python_hh

//---------------------------------------------------------------------------

// This header file provides pybind11 functionality in a py:: namespace along
// with a PYTHON_MODULE macro to be used in the pycpp_XXX compiled python
// modules of the packages. It includes the necessary hacks and workarounds to
// work on the various platforms (try not to put any hacks/workarounds
// elsewhere!).

//---------------------------------------------------------------------------

// We need the same macros here as in Types.hh since pybind headers might include
// the same files and thus spoil Types.hh:

#ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace pyextra {

  //If using python modules from inside regular (libsrc/* or app_*/*) non-python
  //C++ code, one must ensure python has been initialised (for app_*/* code, one
  //is most likely better off using a pybind11 scoped_interpreter).
  inline bool isPyInit() { return Py_IsInitialized(); }

  void pyInit( int argc, char** argv );//Transfer C++ cmdline to sys.argv
  void pyInit( const char * argv0 = nullptr );//only provide sys.argv[0] (defaulting to "dummyargv0")

  inline void ensurePyInit()
  {
    if (!Py_IsInitialized())
      pyInit();
  }

  inline py::object pyimport( const char * name ) { return py::module_::import(name); }
}

//FIXME: Obsolete macro:
#define PYTHON_MODULE3( mod ) PYBIND11_MODULE(PYMODNAME, mod)

#ifdef PYTHON_MODULE
#  undefine PYTHON_MODULE
#endif
#define PYTHON_MODULE( modvarname ) PYBIND11_MODULE(PYMODNAME, modvarname)

//sbld_stringify macro here for convenience:
#ifdef sbld_xstringify
#  undefine sbld_xstringify
#endif
#ifdef sbld_stringify
#  undefine sbld_stringify
#endif
#define sbld_xstringify(s) #s
#define sbld_stringify(s) sbld_xstringify(s)

#endif
