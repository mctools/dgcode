#ifndef Utils_NumpyUtils_hh
#define Utils_NumpyUtils_hh

#include "Core/Python.hh"
#include <stdexcept>
#include <cstddef>
#include <vector>

namespace NumpyUtils {

  //Create and return numpy array object of length n, and let buf point to the
  //internal data storage (which can then be manipulated from C++). Will throw a
  //runtime error if the numpy python module is not available

  py::object create_numpyarray(size_t n, double*& buf);

  //Allow direct access to internal buffer in existing python object. If object
  //has a .data attribute (like numpy arrays do), the access will actually be to
  //the buffer of the .data object.
  void decodeBuffer(py::object o, double*&vals,std::size_t&n);

  //Convert into/from std::vector<double>:
  std::vector<double> pybuf2vectdbl(py::object o);
  py::object vectdbl2numpyarray(const std::vector<double>&);

}

#endif
