#include "Core/Python.hh"
#include "SimpleHistsUtils/Sampler.hh"
#include "SimpleHists/Hist1D.hh"
#include "Utils/NumpyUtils.hh"

namespace SimpleHists_Sampler_py {
  void reinit_1arg(SimpleHists::Sampler* s, SimpleHists::Hist1D*h) { s->reinit(h); }

  py::object sampleMany(SimpleHists::Sampler* s,py::object rand_vals) {
    //Replace rand_vals with sample values and return. This is because I can't
    //figure out how to create a new object without compile-time dependencies on
    //numpy and without memory leaks.
    //TODO: Use new function in NumpyUtils to create the array.
    std::size_t n;
    double * vals;
    NumpyUtils::decodeBuffer(rand_vals,vals,n);
    double * valsE = vals + n;
    for (;vals!=valsE;++vals)
      *vals = s->sample(*vals);
    return rand_vals;
  }
}

PYTHON_MODULE
{
  py::import("SimpleHists");
  py::class_<SimpleHists::Sampler,boost::noncopyable>("Sampler",py::init<SimpleHists::Hist1D*>())
    .def(py::init<SimpleHists::Hist1D*,double>())
    .def("sample",&SimpleHists::Sampler::sample)
    .def("sampleMany",&SimpleHists_Sampler_py::sampleMany)
    .def("__call__",&SimpleHists::Sampler::sample)
    .def("reinit",&SimpleHists::Sampler::reinit)
    .def("reinit",&SimpleHists_Sampler_py::reinit_1arg)
    ;
}
