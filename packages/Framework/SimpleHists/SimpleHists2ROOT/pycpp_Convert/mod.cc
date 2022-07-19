#include "Core/Python.hh"
#include "SimpleHists2ROOT/Convert.hh"
#include "SimpleHists/HistCollection.hh"
#include "TH1D.h"
#include "TH2D.h"

#ifdef HAS_ROOT_PYTHON
#include "TPython.h"
#endif

namespace SimpleHists_pycpp {

  PyObject* convertToPyROOT(const SimpleHists::HistBase*h, const char* root_name)
  {
#ifdef HAS_ROOT_PYTHON
    auto hr = SimpleHists::convertToROOT(h,root_name);
    bool python_owns = false;//NB: We used to have this set to true, but in
                             //ROOTv6 this leads to seg faults. Could
                             //investigate further, but for now we assume we can
                             //live with a few minor possible leaks.
    //NB: Method was called TPython::ObjectProxy_FromVoidPtr before ROOT 6.22.
    return TPython::CPPInstance_FromVoidPtr(hr, hr->ClassName(), python_owns);
#else
    (void)h;
    (void)root_name;
    return 0;
#endif
  }

  PyObject* convertToROOT_1D(const SimpleHists::Hist1D*h, const char* rn) { return convertToPyROOT(h,rn); }
  PyObject* convertToROOT_2D(const SimpleHists::Hist2D*h, const char* rn) { return convertToPyROOT(h,rn); }
  PyObject* convertToROOT_Counts(const SimpleHists::HistCounts*h, const char* rn) { return convertToPyROOT(h,rn); }
  PyObject* convertToROOT_Base(const SimpleHists::HistBase*h, const char* rn) { return convertToPyROOT(h,rn); }

  void convertToROOTFile_hc( const SimpleHists::HistCollection* hc,
                             const char* filename_root )
  {
    SimpleHists::convertToROOTFile(hc,filename_root);
  }

  void convertToROOTFile_fn( const char* filename_shist,
                             const char* filename_root )
  {
    SimpleHists::convertToROOTFile(filename_shist,filename_root);
  }

}

PYTHON_MODULE
{
  py::scope().attr("__doc__") =
    "Python module providing the convertToROOT and convertToROOTFile"
    " functions from Convert.hh. It imports ROOT internally, which can"
    " be avoided by using the sister module " BOOST_STRINGIZE(PACKAGE_NAME) ".ConvertFile"
    " instead, when only file-level manipulations are needed.";

  py::import("ROOT");

  py::def("convertToROOT",&SimpleHists_pycpp::convertToROOT_1D);
  py::def("convertToROOT",&SimpleHists_pycpp::convertToROOT_2D);
  py::def("convertToROOT",&SimpleHists_pycpp::convertToROOT_Counts);
  py::def("convertToROOT",&SimpleHists_pycpp::convertToROOT_Base);
  py::def("convertToROOTFile",&SimpleHists_pycpp::convertToROOTFile_hc);
  py::def("convertToROOTFile",&SimpleHists_pycpp::convertToROOTFile_fn);
}

