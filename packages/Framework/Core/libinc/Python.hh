#ifndef Core_Python_hh
#define Core_Python_hh

//---------------------------------------------------------------------------

//This header file provides pybind11 functionality in a py:: namespace along
//with a PYTHON_MODULE macro to be used in the pycpp_XXX compiled python modules
//of the packages. It includes the necessary hacks and workarounds to work on
//the various platforms (try not to put any hacks/workarounds elsewhere!).

//Obsolete comment to remove after DGCODE_USEPYBIND11 is removed: This header
//file provides boost/python.hpp functionality in a py:: namespace along with a
//PYTHON_MODULE macro to be used in the pycpp_XXX compiled python modules of the
//packages. It includes the necessary hacks and workarounds to work on the
//various platforms (don't put any hacks/workarounds elsewhere!).

//---------------------------------------------------------------------------

//We need the same macros here as in Types.hh since boost/python.hpp will
//include the same files and thus spoil Types.hh:
#ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

#ifdef do_xstringify
#  undefine do_xstringify
#endif
#ifdef do_stringify
#  undefine do_stringify
#endif
#define dg_xstringify(s) #s
#define dg_stringify(s) dg_xstringify(s)

#ifdef DGCODE_USESYSBOOSTPYTHON

#  include <dgboost/python.hpp>
namespace dgboost = boost;
namespace py = boost::python;
#elif defined(DGCODE_USEPYBIND11)
#  include <pybind11/pybind11.h>
namespace py = pybind11;
#else
#  include <dgboost/python.hpp>
namespace py = dgboost::python;
#endif

namespace pyextra {
  bool isPyInit();//whether or not initialisation has been done (same as Py_IsInitialized())
  void ensurePyInit();//call pyInit only if !isPyInit (will initialise with dummy sys.argv[0])
  void pyInit(const char * argv0 = 0);//only provide sys.argv[0] (defaulting to "dummyargv0")
  void pyInit(int argc, char** argv);//Transfer C++ cmdline to sys.argv

#ifdef DGCODE_USEPYBIND11
  inline py::object pyimport( const char * name ) { return py::module_::import(name); }
#else
  inline py::object pyimport( const char * name ) { return py::import(name); }
#endif

}

#ifdef PYDEF
#  undefine PYDEF
#endif
#ifdef PYMOD
#  undefine PYMOD
#endif

#ifdef DGCODE_USEPYBIND11

//TODO
#  define PYTHON_MODULE PYBIND11_MODULE(PYMODNAME, m)
#  define PYTHON_MODULE2 PYBIND11_MODULE(PYMODNAME, themod)
#  define PYDEF m.def
#  define PYMOD m,
#  define PYDEF2 themod.def
#  define PYMOD2 themod,
#  define PYBOOSTNONCOPYABLE
#  define PYBOOSTNOINIT
#  define PYADDPROPERTY def_property
#  define PYADDREADONLYPROPERTY def_property_readonly

namespace pybind11 {

  //Adding this for migration purposes:
  constexpr return_value_policy return_ptr() noexcept { return return_value_policy::reference; }

  //Stop using this, start using pyextra::import (which can also ensurePyInit then!)
  inline py::object pyimport( const char * name )
  {
    return py::module_::import(name);
  }

  //Adding this for migration purposes:
  template <class T>
  inline T extract( py::object o )
  {
    return o.cast<T>();
  }

}
#else

#  define PYDEF py::def
#  define PYMOD
#  define PYDEF2 py::def
#  define PYMOD2
#  define PYBOOSTNONCOPYABLE ,boost::noncopyable
#  define PYBOOSTNOINIT ,py::no_init
#  define PYADDPROPERTY add_property
#  define PYADDREADONLYPROPERTY add_property

#ifndef DGCODE_USESYSBOOSTPYTHON
namespace dgboost {
#else
namespace boost {
#endif

  namespace python {
    typedef return_value_policy<reference_existing_object> return_ptr;

    inline py::object pyimport( const char * name )
    {
      return py::import(name);
    }

    //convenient print + exit on py exception (call within catch statement):
    inline void print_and_handle_exception() {
      PyErr_Print();
      handle_exception();
    }

    template <class T>
    inline py::object dg_ptr2pyobj_refexisting( T* t )
    {
      return py::object(py::detail::new_reference(typename py::reference_existing_object::apply<T *>::type()(t)));
    }

  }
}

//------------------PYTHON_MODULE definition---------------------------------
#define PYTHON_MODULE BOOST_PYTHON_MODULE( PYMODNAME )
#define PYTHON_MODULE2 BOOST_PYTHON_MODULE( PYMODNAME )

#define PYTHON_CREATE_PYOBJECT(T,t) py::object(py::detail::new_reference(typename py::reference_existing_object::apply<T *>::type()(t)));

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Mac osx fix from https://code.google.com/r/przemyslawlinkowski-lightpack/source/browse/Software/PythonQT/src/PythonQtPythonInclude.h?spec=svn46b369b868758cf450d94bf511a0026db6778572&r=46b369b868758cf450d94bf511a0026db6778572:

//
// The following undefs for C standard library macros prevent
// build errors of the following type on mac ox 10.7.4 and XCode 4.3.3
//
// /usr/include/c++/4.2.1/bits/localefwd.h:57:21: error: too many arguments provided to function-like macro invocation
//    isspace(_CharT, const locale&);
//                    ^
// /usr/include/c++/4.2.1/bits/localefwd.h:56:5: error: 'inline' can only appear on functions
//     inline bool
//     ^
// /usr/include/c++/4.2.1/bits/localefwd.h:57:5: error: variable 'isspace' declared as a template
//     isspace(_CharT, const locale&);
//     ^
//
#undef isspace
#undef isupper
#undef islower
#undef isalpha
#undef isalnum
#undef toupper
#undef tolower
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

#endif
