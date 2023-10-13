#include "Core/Python.hh"
#include "SimpleHists/HistBase.hh"
#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCollection.hh"
#include "Utils/NumpyUtils.hh"

#ifdef DGCODE_USEPYBIND11
//  for "py::self += float() etc.
#  include <pybind11/operators.h>
#endif
namespace sh = SimpleHists;

namespace SimpleHists_pycpp {

  void HistBase_dump_0args(const sh::HistBase*h) { h->dump(); }
  void HistBase_dump_1arg(const sh::HistBase*h,bool b) { h->dump(b); }
  int HistBase_histType(const sh::HistBase*h) { return h->histType(); }
  const char* HistBase_getTitle(const sh::HistBase*h) { return h->getTitle().c_str(); }
  const char* HistBase_getXLabel(const sh::HistBase*h) { return h->getXLabel().c_str(); }
  const char* HistBase_getYLabel(const sh::HistBase*h) { return h->getYLabel().c_str(); }
  const char* HistBase_getComment(const sh::HistBase*h) { return h->getComment().c_str(); }

#ifndef DGCODE_USEPYBIND11
  struct HistBase_pickle_suite : py::pickle_suite
  {
    static py::tuple getinitargs(const sh::HistBase& h)
    {
      std::string s;
      h.serialise(s);
      py::object o( py::handle<>( PyBytes_FromStringAndSize( s.c_str(), s.size() ) ) );
      return py::make_tuple(o);
    }
  };
#endif

  void Hist1D_fill_1arg(sh::Hist1D*h,double val) { h->fill(val); }
  void Hist1D_fill_2args(sh::Hist1D*h,double val,double weight) { h->fill(val,weight); }
  void Hist2D_fill_2args(sh::Hist2D*h,double valx,double valy) { h->fill(valx,valy); }
  void Hist2D_fill_3args(sh::Hist2D*h,double valx,double valy,double weight) { h->fill(valx,valy,weight); }

  int Hist1D_getPercentileBin_1arg(sh::Hist1D*h,double arg) { return h->getPercentileBin(arg); }

  py::list HistCol_getKeys(const sh::HistCollection*hc) {
    std::set<std::string> keys;
    hc->getKeys(keys);
    py::list l;
    for ( auto& e : keys )
      l.append( e );
    return l;
  }

  void HistCol_mergeCol(sh::HistCollection*hc,const sh::HistCollection*o) { hc->merge(o); }
  void HistCol_mergeStr(sh::HistCollection*hc,const std::string& s) { hc->merge(s); }
  const char* HistCol_getKey(sh::HistCollection*hc,const sh::HistBase*h) { return hc->getKey(h).c_str(); }

#ifdef DGCODE_USEPYBIND11
  using PyArrayDbl = py::array_t<double,py::array::c_style>;
  void Hist1D_fillFromBuffer_1arg(sh::Hist1D*h, PyArrayDbl py_vals) {
    h->fillMany(py_vals.data(),py_vals.size());
  }
#else
  void Hist1D_fillFromBuffer_1arg(sh::Hist1D*h,py::object py_vals) {
    std::size_t n;
    double * vals;
    NumpyUtils::decodeBuffer(py_vals,vals,n);
    h->fillMany(vals,n);
  }
#endif

#ifdef DGCODE_USEPYBIND11
  void Hist1D_fillFromBuffer_2args(sh::Hist1D*h,PyArrayDbl py_vals,PyArrayDbl py_weights) {
    const auto n = py_vals.size();
    if ( py_weights.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "Value and weights buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(py_vals.data(),py_weights.data(),n);
  }
#else
  void Hist1D_fillFromBuffer_2args(sh::Hist1D*h,py::object py_vals,py::object py_weights) {
    std::size_t nv;
    double * v;
    NumpyUtils::decodeBuffer(py_vals,v,nv);
    std::size_t nw;
    double * w;
    NumpyUtils::decodeBuffer(py_weights,w,nw);
    if (nw!=nv) {
      PyErr_SetString(PyExc_ValueError, "Value and weights buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(v,w,nv);
  }
#endif
#ifdef DGCODE_USEPYBIND11
  void Hist2D_fillFromBuffer_2args(sh::Hist2D*h,PyArrayDbl py_valsx,PyArrayDbl py_valsy) {
    const auto n = py_valsx.size();
    if ( py_valsy.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "X and Y value buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(py_valsx.data(),py_valsy.data(),n);
  }
#else
  void Hist2D_fillFromBuffer_2args(sh::Hist2D*h,py::object py_valsx,py::object py_valsy) {
    std::size_t nvx;
    double * vx;
    NumpyUtils::decodeBuffer(py_valsx,vx,nvx);
    std::size_t nvy;
    double * vy;
    NumpyUtils::decodeBuffer(py_valsy,vy,nvy);
    if (nvx!=nvy) {
      PyErr_SetString(PyExc_ValueError, "X and Y value buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(vx,vy,nvx);
  }
#endif
#ifdef DGCODE_USEPYBIND11
  void Hist2D_fillFromBuffer_3args(sh::Hist2D*h,PyArrayDbl py_valsx,PyArrayDbl py_valsy, PyArrayDbl py_weights) {
    const auto n = py_valsx.size();
    if ( py_valsy.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "X and Y value buffers must have equal length");
      throw py::error_already_set();
    }
    if ( py_weights.size() != n ) {
      PyErr_SetString(PyExc_ValueError, "Value and weights buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(py_valsx.data(),py_valsy.data(),py_weights.data(),n);
  }
#else
  void Hist2D_fillFromBuffer_3args(sh::Hist2D*h,py::object py_valsx,py::object py_valsy, py::object py_weights) {
    std::size_t nvx;
    double * vx;
    NumpyUtils::decodeBuffer(py_valsx,vx,nvx);
    std::size_t nvy;
    double * vy;
    NumpyUtils::decodeBuffer(py_valsy,vy,nvy);
    if (nvx!=nvy) {
      PyErr_SetString(PyExc_ValueError, "X and Y value buffers must have equal length");
      throw py::error_already_set();
    }
    std::size_t nw;
    double * w;
    NumpyUtils::decodeBuffer(py_weights,w,nw);
    if (nvx!=nw) {
      PyErr_SetString(PyExc_ValueError, "Value and weights buffers must have equal length");
      throw py::error_already_set();
    }
    h->fillMany(vx,vy,w,nvx);
  }
#endif

  //HistBase::serialise() must return by val in py interface (less efficient than C++ interface obviously - in C++98):
#ifdef DGCODE_USEPYBIND11
  py::buffer HistBase_serialise(sh::HistBase*h)
#else
  py::object HistBase_serialise(sh::HistBase*h)
#endif
  {
    std::string s;
    h->serialise(s);
#ifndef DGCODE_USEPYBIND11
    py::object o( py::handle<>( PyBytes_FromStringAndSize( s.c_str(), s.size() ) ) );
    return o;
#else
    py::bytes o = s;
    return o;
#endif
  }

  //needed so we can construct efficient numpy arrays of the bin contents:
  py::object Hist1D_rawContentsAsBuffer(sh::Hist1D*h)
  {
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawContents(), h->getNBins()*sizeof(double), PyBUF_READ);
#ifndef DGCODE_USEPYBIND11
    py::object retval = py::object(py::handle<>(py_buf));
#else
    py::object retval = py::reinterpret_borrow<py::object>(py::handle(py_buf));
#endif
    return retval;
  }
  py::object Hist1D_rawErrorsSquaredAsBuffer(sh::Hist1D*h)
  {
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawErrorsSquared(), h->getNBins()*sizeof(double), PyBUF_READ);
#ifndef DGCODE_USEPYBIND11
    py::object retval = py::object(py::handle<>(py_buf));
#else
    py::object retval = py::reinterpret_borrow<py::object>(py::handle(py_buf));
#endif
    return retval;
  }
  py::object Hist2D_rawContentsAsBuffer(sh::Hist2D*h)
  {
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawContents(), h->getNBinsY()*h->getNBinsX()*sizeof(double), PyBUF_READ);
#ifndef DGCODE_USEPYBIND11
    py::object retval = py::object(py::handle<>(py_buf));
#else
    py::object retval = py::reinterpret_borrow<py::object>(py::handle(py_buf));
#endif
    return retval;
  }

  sh::Hist1D* HistCol_book1Dv1(sh::HistCollection* hc,
                               unsigned nbins, double xmin, double xmax, const std::string& key)
  {
    return hc->book1D(nbins,xmin,xmax,key);
  }

  sh::Hist1D* HistCol_book1Dv2(sh::HistCollection* hc,
                               const std::string& title,
                               unsigned nbins, double xmin, double xmax, const std::string& key)
  {
    return hc->book1D(title,nbins,xmin,xmax,key);
  }

  sh::Hist2D* HistCol_book2Dv1(sh::HistCollection* hc,
                               unsigned nbinsx, double xmin, double xmax,
                               unsigned nbinsy, double ymin, double ymax, const std::string& key)
  {
    return hc->book2D(nbinsx,xmin,xmax,nbinsy,ymin,ymax,key);
  }

  sh::Hist2D* HistCol_book2Dv2(sh::HistCollection* hc,
                               const std::string& title,
                               unsigned nbinsx, double xmin, double xmax,
                               unsigned nbinsy, double ymin, double ymax, const std::string& key)
  {
    return hc->book2D(title,nbinsx,xmin,xmax,nbinsy,ymin,ymax,key);
  }

  sh::HistCounts* HistCol_bookCountsv1(sh::HistCollection* hc,const std::string& key)
  {
    return hc->bookCounts(key);
  }

  sh::HistCounts* HistCol_bookCountsv2(sh::HistCollection* hc,const std::string& title,const std::string& key)
  {
    return hc->bookCounts(title,key);
  }

  sh::HistBase* HistCol_histnonconst(sh::HistCollection* hc, const std::string& key)
  {
    return hc->hist(key);
  }
  const sh::HistBase* HistCol_histconst(const sh::HistCollection* hc, const std::string& key)
  {
    return hc->hist(key);
  }

  void HistColl_saveToFile_1arg(const sh::HistCollection* hc, const std::string& f)
  {
    hc->saveToFile(f);
  }

  sh::HistCounts::Counter HistCounts_addCounter_1arg(sh::HistCounts* hc, const std::string& l)
  {
    return hc->addCounter(l);
  }

  const char * Counter_getLabel(sh::HistCounts::Counter* c) { return c->getLabel().c_str(); }
  const char * Counter_getDisplayLabel(sh::HistCounts::Counter* c) { return c->getDisplayLabel().c_str(); }
  const char * Counter_getComment(sh::HistCounts::Counter* c) { return c->getComment().c_str(); }


  py::list HistCounts_getCounters(sh::HistCounts*h)
  {
    std::list<sh::HistCounts::Counter> l;
    h->getCounters(l);

    py::list res;

    auto itE=l.end();
    for (auto it=l.begin();it!=itE;++it) {
      res.append(*it);
    }
    return res;
  }

  sh::HistCounts::Counter HistCounts_getCounter(sh::HistCounts*h, const std::string& l)
  {
    return h->getCounter(l);
  }

}

namespace shp = SimpleHists_pycpp;

PYTHON_MODULE
{
  //Change the module name to avoid type(Hist1D()) giving
  //'SimpleHists._init.Hist1D' but rather give 'SimpleHists.Hist1D':
#ifndef DGCODE_USEPYBIND11
  py::scope().attr("__name__") = BOOST_STRINGIZE(PACKAGE_NAME);
#else

#define simplehist_str(s) #s
#define simplehist_xstr(s) simplehist_str(s)
  m.attr("__name__") = simplehist_xstr(PACKAGE_NAME);
#endif
#ifdef DGCODE_USEPYBIND11
  py::class_<sh::HistBase PYBOOSTNONCOPYABLE> thePyHistBaseClass(PYMOD "HistBase" PYBOOSTNOINIT);
  thePyHistBaseClass
#else
  py::class_<sh::HistBase PYBOOSTNONCOPYABLE>("HistBase" PYBOOSTNOINIT)
#endif
    .def("getTitle",&shp::HistBase_getTitle)
    .def("getXLabel",&shp::HistBase_getXLabel)
    .def("getYLabel",&shp::HistBase_getYLabel)
    .def("getComment",&shp::HistBase_getComment)
    .def("setTitle",&sh::HistBase::setTitle)
    .def("setXLabel",&sh::HistBase::setXLabel)
    .def("setYLabel",&sh::HistBase::setYLabel)
    .def("setComment",&sh::HistBase::setComment)
    .def("histType",&shp::HistBase_histType)
    .def("getIntegral",&sh::HistBase::getIntegral)
    .def("empty",&sh::HistBase::empty)
    .def("dimension",&sh::HistBase::dimension)
    .def("serialise",&shp::HistBase_serialise)
    .def("dump",&sh::HistBase::dump)//version with 2 args
    .def("dump",&shp::HistBase_dump_1arg)//version with 1 arg
    .def("dump",&shp::HistBase_dump_0args)//version with 0 args
    .def("mergeCompatible",&sh::HistBase::mergeCompatible)
    .def("merge",&sh::HistBase::merge)
    .def("isSimilar",&sh::HistBase::isSimilar)
    .def("scale",&sh::HistBase::scale)
    .def("norm",&sh::HistBase::norm)
    .def("clone",&sh::HistBase::clone,py::return_ptr())
    .def("reset",&sh::HistBase::reset)
#ifdef DGCODE_USEPYBIND11
#else
    // //Define legal combination of the special attributes and methods:
    // //__getinitargs__, __getstate__, __setstate__, __getstate_manages_dict__, __safe_for_unpickling__, __reduce__
    // //Providing full pickle support.
    .def_pickle(shp::HistBase_pickle_suite())
#endif
    //properties (all lowercase):
    .PYADDPROPERTY("title", &shp::HistBase_getTitle,&sh::HistBase::setTitle)
    .PYADDPROPERTY("xlabel", &shp::HistBase_getXLabel, &sh::HistBase::setXLabel)
    .PYADDPROPERTY("ylabel", &shp::HistBase_getYLabel, &sh::HistBase::setYLabel)
    .PYADDPROPERTY("comment", &shp::HistBase_getComment, &sh::HistBase::setComment)
    .PYADDREADONLYPROPERTY("integral",&sh::HistBase::getIntegral)
    ;

  PYDEF("histTypeOfData",&sh::histTypeOfData);
  PYDEF("deserialise",&sh::deserialise,py::return_ptr());
#ifdef DGCODE_USEPYBIND11
  PYDEF("deserialiseAndManage",&sh::deserialise,py::return_value_policy::take_ownership);
#else
  PYDEF("deserialiseAndManage",&sh::deserialise,py::return_value_policy<py::manage_new_object>());
#endif

  //Hist1D:
#ifdef DGCODE_USEPYBIND11
  py::class_<sh::Hist1D> thePyHist1DClass(m, "Hist1D", thePyHistBaseClass);
  thePyHist1DClass
    .def(py::init<unsigned, double, double>(),py::arg("nbins"),py::arg("xmin"),py::arg("xmax"))
    .def(py::init<const std::string&,unsigned, double, double>(),py::arg("title"),py::arg("nbins"),py::arg("xmin"),py::arg("xmax"))
    .def(py::init([]( py::buffer arg) { return std::make_unique<sh::Hist1D>(py::cast<std::string>(arg));}), py::arg("serialised_data") )
    .def(py::pickle(
                    [](const sh::Hist1D& h) {
                      std::string s;
                      h.serialise(s);
                      py::bytes bytes(s);
                      return py::make_tuple(bytes);
                    },
                    [](py::tuple t) {
                      if (t.size() != 1)
                        throw std::runtime_error("Invalid state!");
                      std::string serialised_data = t[0].cast<std::string>();
                      if (sh::histTypeOfData(serialised_data)!=0x01)
                        throw std::runtime_error("Invalid state!");
                      return std::make_unique<sh::Hist1D>(serialised_data);
                    }))
#else
  py::class_<sh::Hist1D PYBOOSTNONCOPYABLE , py::bases<sh::HistBase> >(PYMOD "Hist1D",py::init<unsigned, double, double>())
    .def(py::init<const std::string&,unsigned, double, double>())
    .def(py::init<const std::string&>())
#endif
    .def("getNBins",&sh::Hist1D::getNBins)
    .def("getBinContent",&sh::Hist1D::getBinContent)
    .def("getBinError",&sh::Hist1D::getBinError)
    .def("getBinCenter",&sh::Hist1D::getBinCenter)
    .def("getBinLower",&sh::Hist1D::getBinLower)
    .def("getBinUpper",&sh::Hist1D::getBinUpper)
    .def("getBinWidth",&sh::Hist1D::getBinWidth)
    .def("getMaxContent",&sh::Hist1D::getMaxContent)
    .def("getUnderflow",&sh::Hist1D::getUnderflow)
    .def("getOverflow",&sh::Hist1D::getOverflow)
    .def("getMinFilled",&sh::Hist1D::getMinFilled)
    .def("getMaxFilled",&sh::Hist1D::getMaxFilled)
    .def("getXMin",&sh::Hist1D::getXMin)
    .def("getXMax",&sh::Hist1D::getXMax)
    .def("getMean",&sh::Hist1D::getMean)
    .def("getRMS",&sh::Hist1D::getRMS)
    .def("getRMSSquared",&sh::Hist1D::getRMSSquared)
    .def("valueToBin",&sh::Hist1D::valueToBin)
    .def("fillN",&sh::Hist1D::fillN)
    .def("_rawfill",&shp::Hist1D_fill_1arg)
    .def("_rawfill",&shp::Hist1D_fill_2args)
    .def("_rawfillFromBuffer",&shp::Hist1D_fillFromBuffer_1arg)
    .def("_rawfillFromBuffer",&shp::Hist1D_fillFromBuffer_2args)
    .def("_rawContents",&shp::Hist1D_rawContentsAsBuffer)
    .def("_rawErrorsSquared",&shp::Hist1D_rawErrorsSquaredAsBuffer)
    .def("setErrorsByContent",&sh::Hist1D::setErrorsByContent)
    .def("rebin",&sh::Hist1D::rebin)
    .def("canRebin",&sh::Hist1D::canRebin)
    .def("resetAndRebin",&sh::Hist1D::resetAndRebin)
    .def("getPercentileBin",&sh::Hist1D::getPercentileBin)
    .def("getPercentileBin",&shp::Hist1D_getPercentileBin_1arg)
    .def("getBinSum",&sh::Hist1D::getBinSum)
    //readonly properties (all lowercase):
    .PYADDREADONLYPROPERTY("nbins",&sh::Hist1D::getNBins)
    .PYADDREADONLYPROPERTY("binwidth",&sh::Hist1D::getBinWidth)
    .PYADDREADONLYPROPERTY("underflow",&sh::Hist1D::getUnderflow)
    .PYADDREADONLYPROPERTY("overflow",&sh::Hist1D::getOverflow)
    .PYADDREADONLYPROPERTY("minfilled",&sh::Hist1D::getMinFilled)
    .PYADDREADONLYPROPERTY("maxfilled",&sh::Hist1D::getMaxFilled)
    .PYADDREADONLYPROPERTY("xmin",&sh::Hist1D::getXMin)
    .PYADDREADONLYPROPERTY("xmax",&sh::Hist1D::getXMax)
    .PYADDREADONLYPROPERTY("mean",&sh::Hist1D::getMean)
    .PYADDREADONLYPROPERTY("rms",&sh::Hist1D::getRMS)
    .PYADDREADONLYPROPERTY("rms2",&sh::Hist1D::getRMSSquared)
    .PYADDREADONLYPROPERTY("maxcontent",&sh::Hist1D::getMaxContent)
    ;

  //Hist2D:
#ifdef DGCODE_USEPYBIND11
  py::class_<sh::Hist2D> thePyHist2DClass(m,"Hist2D",thePyHistBaseClass);
  thePyHist2DClass
    .def(py::init<unsigned, double, double, unsigned, double, double>(),
         py::arg("nbinsx"),py::arg("xmin"),py::arg("xmax"),
         py::arg("nbinsy"),py::arg("ymin"),py::arg("ymax"))
    .def(py::init<const std::string&,unsigned, double, double,unsigned, double, double>(),
         py::arg("title"),py::arg("nbinsx"),py::arg("xmin"),py::arg("xmax"),
         py::arg("nbinsy"),py::arg("ymin"),py::arg("ymax"))
    .def(py::init([]( py::buffer arg) { return std::make_unique<sh::Hist2D>(py::cast<std::string>(arg));}), py::arg("serialised_data") )
    .def(py::pickle(
                    [](const sh::Hist2D& h) {
                      std::string s;
                      h.serialise(s);
                      py::bytes bytes(s);
                      return py::make_tuple(bytes);
                    },
                    [](py::tuple t) {
                      if (t.size() != 1)
                        throw std::runtime_error("Invalid state!");
                      std::string serialised_data = t[0].cast<std::string>();
                      if (sh::histTypeOfData(serialised_data)!=0x02)
                        throw std::runtime_error("Invalid state!");
                      return std::make_unique<sh::Hist2D>(serialised_data);
                    }))
#else
  py::class_<sh::Hist2D PYBOOSTNONCOPYABLE , py::bases<sh::HistBase> >(PYMOD "Hist2D",py::init<unsigned, double, double, unsigned, double, double>())
    .def(py::init<const std::string&,unsigned, double, double,unsigned, double, double>())
    .def(py::init<const std::string&>())
#endif
    .def("getNBinsX",&sh::Hist2D::getNBinsX)
    .def("getNBinsY",&sh::Hist2D::getNBinsY)
    .def("getBinContent",&sh::Hist2D::getBinContent)
    .def("getBinCenterX",&sh::Hist2D::getBinCenterX)
    .def("getBinLowerX",&sh::Hist2D::getBinLowerX)
    .def("getBinUpperX",&sh::Hist2D::getBinUpperX)
    .def("getBinCenterY",&sh::Hist2D::getBinCenterY)
    .def("getBinLowerY",&sh::Hist2D::getBinLowerY)
    .def("getBinUpperY",&sh::Hist2D::getBinUpperY)
    .def("getBinWidthX",&sh::Hist2D::getBinWidthX)
    .def("getBinWidthY",&sh::Hist2D::getBinWidthY)
    .def("getUnderflowX",&sh::Hist2D::getUnderflowX)
    .def("getOverflowY",&sh::Hist2D::getOverflowY)
    .def("getUnderflowX",&sh::Hist2D::getUnderflowX)
    .def("getOverflowY",&sh::Hist2D::getOverflowY)
    .def("getMinFilledX",&sh::Hist2D::getMinFilledX)
    .def("getMaxFilledX",&sh::Hist2D::getMaxFilledX)
    .def("getMinFilledY",&sh::Hist2D::getMinFilledY)
    .def("getMaxFilledY",&sh::Hist2D::getMaxFilledY)
    .def("getXMin",&sh::Hist2D::getXMin)
    .def("getXMax",&sh::Hist2D::getXMax)
    .def("getYMin",&sh::Hist2D::getYMin)
    .def("getYMax",&sh::Hist2D::getYMax)
    .def("getMeanX",&sh::Hist2D::getMeanX)
    .def("getRMSX",&sh::Hist2D::getRMSX)
    .def("getRMSSquaredX",&sh::Hist2D::getRMSSquaredX)
    .def("getMeanY",&sh::Hist2D::getMeanY)
    .def("getRMSY",&sh::Hist2D::getRMSY)
    .def("getRMSSquaredY",&sh::Hist2D::getRMSSquaredY)
    .def("getCovariance",&sh::Hist2D::getCovariance)
    .def("getCorrelation",&sh::Hist2D::getCorrelation)
    .def("valueToBinX",&sh::Hist2D::valueToBinX)
    .def("valueToBinY",&sh::Hist2D::valueToBinY)
    .def("_rawfill",&shp::Hist2D_fill_2args)
    .def("_rawfill",&shp::Hist2D_fill_3args)
    .def("_rawfillFromBuffer",&shp::Hist2D_fillFromBuffer_2args)
    .def("_rawfillFromBuffer",&shp::Hist2D_fillFromBuffer_3args)
    .def("_rawContents",&shp::Hist2D_rawContentsAsBuffer)
    //readonly properties (all lowercase):
    .PYADDREADONLYPROPERTY("binwidthx",&sh::Hist2D::getBinWidthX)
    .PYADDREADONLYPROPERTY("binwidthy",&sh::Hist2D::getBinWidthY)
    .PYADDREADONLYPROPERTY("nbinsx",&sh::Hist2D::getNBinsX)
    .PYADDREADONLYPROPERTY("nbinsy",&sh::Hist2D::getNBinsY)
    .PYADDREADONLYPROPERTY("underflowx",&sh::Hist2D::getUnderflowX)
    .PYADDREADONLYPROPERTY("overflowx",&sh::Hist2D::getOverflowX)
    .PYADDREADONLYPROPERTY("underflowy",&sh::Hist2D::getUnderflowY)
    .PYADDREADONLYPROPERTY("overflowy",&sh::Hist2D::getOverflowY)
    .PYADDREADONLYPROPERTY("minfilledx",&sh::Hist2D::getMinFilledX)
    .PYADDREADONLYPROPERTY("maxfilledx",&sh::Hist2D::getMaxFilledX)
    .PYADDREADONLYPROPERTY("minfilledy",&sh::Hist2D::getMinFilledY)
    .PYADDREADONLYPROPERTY("maxfilledy",&sh::Hist2D::getMaxFilledY)
    .PYADDREADONLYPROPERTY("xmin",&sh::Hist2D::getXMin)
    .PYADDREADONLYPROPERTY("xmax",&sh::Hist2D::getXMax)
    .PYADDREADONLYPROPERTY("ymin",&sh::Hist2D::getYMin)
    .PYADDREADONLYPROPERTY("ymax",&sh::Hist2D::getYMax)
    .PYADDREADONLYPROPERTY("meanx",&sh::Hist2D::getMeanX)
    .PYADDREADONLYPROPERTY("rmsx",&sh::Hist2D::getRMSX)
    .PYADDREADONLYPROPERTY("rms2x",&sh::Hist2D::getRMSSquaredX)
    .PYADDREADONLYPROPERTY("meany",&sh::Hist2D::getMeanY)
    .PYADDREADONLYPROPERTY("rmsy",&sh::Hist2D::getRMSY)
    .PYADDREADONLYPROPERTY("rms2y",&sh::Hist2D::getRMSSquaredY)
    .PYADDREADONLYPROPERTY("covariance",&sh::Hist2D::getCovariance)
    .PYADDREADONLYPROPERTY("covxy",&sh::Hist2D::getCovariance)
    .PYADDREADONLYPROPERTY("correlation",&sh::Hist2D::getCorrelation)
    .PYADDREADONLYPROPERTY("corxy",&sh::Hist2D::getCorrelation)
    ;

  //HistCounts::Counter:
  py::class_<sh::HistCounts::Counter>(PYMOD "Counter" PYBOOSTNOINIT)
    .def("getValue",&sh::HistCounts::Counter::getValue)
    .def("__call__",&sh::HistCounts::Counter::getValue)
    .def("getError",&sh::HistCounts::Counter::getError)
    .def("getErrorSquared",&sh::HistCounts::Counter::getErrorSquared)
    .def("getLabel",&shp::Counter_getLabel)
    .def("getDisplayLabel",&shp::Counter_getDisplayLabel)
    .def("setDisplayLabel",&sh::HistCounts::Counter::setDisplayLabel)
    .def("getComment",&shp::Counter_getComment)
    .def("setComment",&sh::HistCounts::Counter::setComment)
    .def(py::self += float())
    .def(py::self += py::self)
    //readonly properties (all lowercase):
    .PYADDREADONLYPROPERTY("value",&sh::HistCounts::Counter::getValue)
    .PYADDREADONLYPROPERTY("error",&sh::HistCounts::Counter::getError)
    .PYADDREADONLYPROPERTY("errorsquared",&sh::HistCounts::Counter::getErrorSquared)
    .PYADDREADONLYPROPERTY("label",&shp::Counter_getLabel)
    .PYADDPROPERTY("displaylabel",&shp::Counter_getDisplayLabel,&sh::HistCounts::Counter::setDisplayLabel)
    .PYADDPROPERTY("comment",&shp::Counter_getComment,&sh::HistCounts::Counter::setComment)
    ;

  //HistCounts:
#ifdef DGCODE_USEPYBIND11
  py::class_<sh::HistCounts> thePyHistCountsClass(m, "HistCounts",thePyHistBaseClass);
  thePyHistCountsClass
    .def(py::init<>())
    .def(py::init([]( py::str arg) { auto h = std::make_unique<sh::HistCounts>(); h->setTitle( py::cast<std::string>(arg) ); return h; }), py::arg("title") )
    .def(py::init([]( py::buffer arg) { return std::make_unique<sh::HistCounts>(py::cast<std::string>(arg));}), py::arg("serialised_data") )
    .def(py::pickle(
                    [](const sh::HistCounts& h) {
                      std::string s;
                      h.serialise(s);
                      py::bytes bytes(s);
                      return py::make_tuple(bytes);
                    },
                    [](py::tuple t) {
                      if (t.size() != 1)
                        throw std::runtime_error("Invalid state!");
                      std::string serialised_data = t[0].cast<std::string>();
                      if (sh::histTypeOfData(serialised_data)!=0x03)
                        throw std::runtime_error("Invalid state!");
                      return std::make_unique<sh::HistCounts>(serialised_data);
                    }))
#else
  py::class_<sh::HistCounts PYBOOSTNONCOPYABLE , py::bases<sh::HistBase> >(PYMOD "HistCounts",py::init<>())
    .def(py::init<const std::string&>())
#endif
    .def("addCounter",&sh::HistCounts::addCounter)
    .def("addCounter",&shp::HistCounts_addCounter_1arg)
    .def("getMaxContent",&sh::HistCounts::getMaxContent)
    .def("setErrorsByContent",&sh::HistCounts::setErrorsByContent)
    .def("sortByLabels",&sh::HistCounts::sortByLabels)
    .def("sortByDisplayLabels",&sh::HistCounts::sortByDisplayLabels)
    .def("getCounters",&shp::HistCounts_getCounters)
    .def("getCounter",&shp::HistCounts_getCounter)
    .def("hasCounter",&sh::HistCounts::hasCounter)
    .def("nCounters",&sh::HistCounts::nCounters)
    //readonly properties (all lowercase):
    .PYADDREADONLYPROPERTY("ncounters",&sh::HistCounts::nCounters)
    .PYADDREADONLYPROPERTY("maxcontent",&sh::HistCounts::getMaxContent)
    .PYADDREADONLYPROPERTY("counters",&shp::HistCounts_getCounters)
    ;

  //HistCollection:
#ifdef DGCODE_USEPYBIND11
  py::class_<sh::HistCollection>(m,"HistCollection")
    .def(py::init<>())
    .def(py::init<const std::string&>(),py::arg("filename"))
#else
  py::class_<sh::HistCollection PYBOOSTNONCOPYABLE>(PYMOD "HistCollection",py::init<>())
    .def(py::init<const std::string&>())
#endif
    .def("book1D",&shp::HistCol_book1Dv1,py::return_ptr())
    .def("book1D",&shp::HistCol_book1Dv2,py::return_ptr())
    .def("book2D",&shp::HistCol_book2Dv1,py::return_ptr())
    .def("book2D",&shp::HistCol_book2Dv2,py::return_ptr())
    .def("bookCounts",&shp::HistCol_bookCountsv1,py::return_ptr())
    .def("bookCounts",&shp::HistCol_bookCountsv2,py::return_ptr())
    .def("hasKey",&sh::HistCollection::hasKey)
    .def("getKey",&shp::HistCol_getKey)
    .def("hist",&shp::HistCol_histnonconst,py::return_ptr())
    .def("hist",&shp::HistCol_histconst,py::return_ptr())
    .def("add",&sh::HistCollection::add)
    .def("remove",&sh::HistCollection::remove,py::return_ptr())
#ifdef DGCODE_USEPYBIND11
    .def("removeAndManage",&sh::HistCollection::remove,py::return_value_policy::take_ownership)
#else
    .def("removeAndManage",&sh::HistCollection::remove,py::return_value_policy<py::manage_new_object>())
#endif
    .def("saveToFile",&sh::HistCollection::saveToFile)
    .def("saveToFile",&shp::HistColl_saveToFile_1arg)
    .def("getKeys",&shp::HistCol_getKeys)
    .def("merge",&shp::HistCol_mergeCol)
    .def("merge",&shp::HistCol_mergeStr)
    .def("isSimilar",&sh::HistCollection::isSimilar)
    .PYADDREADONLYPROPERTY("keys",&shp::HistCol_getKeys)
    ;

}

