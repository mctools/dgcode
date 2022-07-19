#include "Utils/NumpyUtils.hh"
#include <cassert>
#include <cstring>

py::object NumpyUtils::create_numpyarray(size_t n, double*& buf)
{
  buf = 0;
  //Create numpy arrays for results:
  py::object numpy;
  try {
    numpy = py::import("numpy");
  } catch (const boost::python::error_already_set &) {
    numpy = py::object();
  }
  if (!numpy)
    throw std::runtime_error("Failure to import python module 'numpy'");
  py::object nparr = numpy.attr("zeros")(n,"float64");
  //Extract internal buffer of numpy array for direct C++ access:
  py::object npdata = nparr.attr("data");
  PyObject* rawdata = npdata.ptr();
#if PY_MAJOR_VERSION<3
  //python2
  assert( PyBuffer_Check(rawdata) );
  assert( (size_t)PySequence_Length(rawdata) == n * sizeof(double) );
  PyBufferProcs * bufferProcs = rawdata->ob_type->tp_as_buffer;
  (*bufferProcs->bf_getreadbuffer)(rawdata, 0, (void **) &buf);
#else
  //python3
  Py_buffer view;
  assert( (size_t)PySequence_Length(rawdata) == n );
  if ( PyObject_GetBuffer(rawdata, &view,  PyBUF_FULL) != 0 )
    throw std::runtime_error("Failure to acquire numpy buffer");
  if (!PyBuffer_IsContiguous(&view,'C') )
    throw std::runtime_error("Numpy buffer not contiguous");
  if ( view.len != (Py_ssize_t)(n * sizeof(double)) )
    throw std::runtime_error("Numpy buffer has wrong length");
  buf = (double*)view.buf;;
  PyBuffer_Release(&view);//decrement ref-count
#endif
  assert(buf);
  return nparr;
}

void NumpyUtils::decodeBuffer(py::object py_obj, double*&vals,std::size_t&n) {
  //Get raw pointer to py_obj.data if there is a .data attribute (this is where
  //a numpy array keeps its buffer), else to the py_obj itself which is assumed
  //to be a buffer:
  py::object pydataobj = py::getattr(py_obj,"data",py_obj);
  PyObject* pydataobj_cptr = pydataobj.ptr();
#if PY_MAJOR_VERSION<3
  //python2
  if (!PyBuffer_Check(pydataobj_cptr)) {
    PyErr_SetString(PyExc_TypeError, "Expected buffer object");
    throw py::error_already_set();
  }
  n = PySequence_Length(pydataobj_cptr)/sizeof(double);
  PyBufferProcs * bufferProcs = pydataobj_cptr->ob_type->tp_as_buffer;
  (*bufferProcs->bf_getreadbuffer)(pydataobj_cptr, 0, (void **) &vals);
#else
  //python3
  Py_buffer view;
  if ( PyObject_GetBuffer(pydataobj_cptr, &view,  PyBUF_FULL) != 0 )
    throw std::runtime_error("Failure to acquire numpy buffer");
  if (!PyBuffer_IsContiguous(&view,'C') )
    throw std::runtime_error("Numpy buffer not contiguous");

  //Figure out expected length:
  py::object pysizeobj = py::getattr(py_obj,"size",py::object());
  if (pysizeobj) {
    //numpy objects keep total number of entries (aka the product of length in
    //each dimension as specified in the .shape) in .size:
    n = py::extract<std::size_t>(pysizeobj);
  } else {
    //Just take the len(pydataobj):
    n = PySequence_Length(pydataobj_cptr);
  }
  if ( view.len != (Py_ssize_t)(n*sizeof(double)) )
    throw std::runtime_error("Numpy buffer has wrong length");
  vals = (double*)view.buf;;
  PyBuffer_Release(&view);//decrement ref-count
#endif
}

std::vector<double> NumpyUtils::pybuf2vectdbl(py::object o) {
  double * data;
  std::size_t n;
  NumpyUtils::decodeBuffer(o, data, n);
  return std::vector<double>(data,data+n);
}

py::object NumpyUtils::vectdbl2numpyarray(const std::vector<double>& v)
{
  double * buf;
  py::object o = NumpyUtils::create_numpyarray(v.size(), buf);
  if (!v.empty())
    std::memcpy(buf,&v[0],v.size()*sizeof(double));
  return o;
}
