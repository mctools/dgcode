#include "Core/Python.hh"
#include "SimpleHists/HistBase.hh"
#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCollection.hh"
#include "Utils/NumpyUtils.hh"

namespace sh = SimpleHists;

namespace SimpleHists_pycpp {

  void HistBase_dump_0args(const sh::HistBase*h) { h->dump(); }
  void HistBase_dump_1arg(const sh::HistBase*h,bool b) { h->dump(b); }
  int HistBase_histType(const sh::HistBase*h) { return h->histType(); }
  const char* HistBase_getTitle(const sh::HistBase*h) { return h->getTitle().c_str(); }
  const char* HistBase_getXLabel(const sh::HistBase*h) { return h->getXLabel().c_str(); }
  const char* HistBase_getYLabel(const sh::HistBase*h) { return h->getYLabel().c_str(); }
  const char* HistBase_getComment(const sh::HistBase*h) { return h->getComment().c_str(); }

  struct HistBase_pickle_suite : py::pickle_suite
  {
    static py::tuple getinitargs(const sh::HistBase& h)
    {
      std::string s;
      h.serialise(s);
      py::object o( py::handle<>( PyBytes_FromStringAndSize( s.c_str(), s.size() ) ) );
      return py::make_tuple(o);
      //return o;
      //std::string s;
      //h.serialise(s);
      //return py::make_tuple(s);
    }
  };

  void Hist1D_fill_1arg(sh::Hist1D*h,double val) { h->fill(val); }
  void Hist1D_fill_2args(sh::Hist1D*h,double val,double weight) { h->fill(val,weight); }
  void Hist2D_fill_2args(sh::Hist2D*h,double valx,double valy) { h->fill(valx,valy); }
  void Hist2D_fill_3args(sh::Hist2D*h,double valx,double valy,double weight) { h->fill(valx,valy,weight); }

  int Hist1D_getPercentileBin_1arg(sh::Hist1D*h,double arg) { return h->getPercentileBin(arg); }

  py::list HistCol_getKeys(const sh::HistCollection*hc) {
    std::set<std::string> keys;
    hc->getKeys(keys);
    return py::stdcol2pylist(keys);
  }

  void HistCol_mergeCol(sh::HistCollection*hc,const sh::HistCollection*o) { hc->merge(o); }
  void HistCol_mergeStr(sh::HistCollection*hc,const std::string& s) { hc->merge(s); }
  const char* HistCol_getKey(sh::HistCollection*hc,const sh::HistBase*h) { return hc->getKey(h).c_str(); }

  void Hist1D_fillFromBuffer_1arg(sh::Hist1D*h,py::object py_vals) {
    std::size_t n;
    double * vals;
    NumpyUtils::decodeBuffer(py_vals,vals,n);
    h->fillMany(vals,n);
  }
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

  //HistBase::serialise() must return by val in py interface (less efficient than C++ interface obviously - in C++98):
  //  std::string HistBase_serialise(sh::HistBase*h) { std::string s; h->serialise(s); return s; }
  py::object HistBase_serialise(sh::HistBase*h)
  {
    std::string s;
    h->serialise(s);
    py::object o( py::handle<>( PyBytes_FromStringAndSize( s.c_str(), s.size() ) ) );
    return o;
  }

  //needed so we can construct efficient numpy arrays of the bin contents:
  py::object Hist1D_rawContentsAsBuffer(sh::Hist1D*h)
  {
#if PY_MAJOR_VERSION<3
    //python2
    PyObject* py_buf = PyBuffer_FromMemory((void*)h->rawContents(), h->getNBins()*sizeof(double));//readonly buffer
#else
    //python3
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawContents(), h->getNBins()*sizeof(double), PyBUF_READ);
#endif
    py::object retval = py::object(py::handle<>(py_buf));
    return retval;
  }
  py::object Hist1D_rawErrorsSquaredAsBuffer(sh::Hist1D*h)
  {
#if PY_MAJOR_VERSION<3
    //python2
    PyObject* py_buf = PyBuffer_FromMemory((void*)h->rawErrorsSquared(), h->getNBins()*sizeof(double));//readonly buffer
#else
    //python3
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawErrorsSquared(), h->getNBins()*sizeof(double), PyBUF_READ);
#endif
    py::object retval = py::object(py::handle<>(py_buf));
    return retval;
  }
  py::object Hist2D_rawContentsAsBuffer(sh::Hist2D*h)
  {
#if PY_MAJOR_VERSION<3
    //python2
    PyObject* py_buf = PyBuffer_FromMemory((void*)h->rawContents(), h->getNBinsY()*h->getNBinsX()*sizeof(double));//readonly buffer
#else
    //python3
    PyObject* py_buf = PyMemoryView_FromMemory((char*)h->rawContents(), h->getNBinsY()*h->getNBinsX()*sizeof(double), PyBUF_READ);
#endif
    py::object retval = py::object(py::handle<>(py_buf));
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
  py::scope().attr("__name__") = BOOST_STRINGIZE(PACKAGE_NAME);

  py::class_<sh::HistBase,boost::noncopyable>("HistBase",py::no_init)
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
    // //Define legal combination of the special attributes and methods:
    // //__getinitargs__, __getstate__, __setstate__, __getstate_manages_dict__, __safe_for_unpickling__, __reduce__
    // //Providing full pickle support.
    .def_pickle(shp::HistBase_pickle_suite())
    //properties (all lowercase):
    .add_property("title", &shp::HistBase_getTitle,&sh::HistBase::setTitle)
    .add_property("xlabel", &shp::HistBase_getXLabel, &sh::HistBase::setXLabel)
    .add_property("ylabel", &shp::HistBase_getYLabel, &sh::HistBase::setYLabel)
    .add_property("comment", &shp::HistBase_getComment, &sh::HistBase::setComment)
    .add_property("integral",&sh::HistBase::getIntegral)
    ;

  py::def("histTypeOfData",&sh::histTypeOfData);
  py::def("deserialise",&sh::deserialise,py::return_ptr());
  py::def("deserialiseAndManage",&sh::deserialise,py::return_value_policy<py::manage_new_object>());

  //Hist1D:
  py::class_<sh::Hist1D,boost::noncopyable,py::bases<sh::HistBase> >("Hist1D",py::init<unsigned, double, double>())
    .def(py::init<const std::string&,unsigned, double, double>())
    .def(py::init<const std::string&>())
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
    .add_property("nbins",&sh::Hist1D::getNBins)
    .add_property("binwidth",&sh::Hist1D::getBinWidth)
    .add_property("underflow",&sh::Hist1D::getUnderflow)
    .add_property("overflow",&sh::Hist1D::getOverflow)
    .add_property("minfilled",&sh::Hist1D::getMinFilled)
    .add_property("maxfilled",&sh::Hist1D::getMaxFilled)
    .add_property("xmin",&sh::Hist1D::getXMin)
    .add_property("xmax",&sh::Hist1D::getXMax)
    .add_property("mean",&sh::Hist1D::getMean)
    .add_property("rms",&sh::Hist1D::getRMS)
    .add_property("rms2",&sh::Hist1D::getRMSSquared)
    .add_property("maxcontent",&sh::Hist1D::getMaxContent)
    ;

  //Hist2D:
  py::class_<sh::Hist2D,boost::noncopyable,py::bases<sh::HistBase> >("Hist2D",py::init<unsigned, double, double, unsigned, double, double>())
    .def(py::init<const std::string&,unsigned, double, double,unsigned, double, double>())
    .def(py::init<const std::string&>())
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
    .add_property("binwidthx",&sh::Hist2D::getBinWidthX)
    .add_property("binwidthy",&sh::Hist2D::getBinWidthY)
    .add_property("nbinsx",&sh::Hist2D::getNBinsX)
    .add_property("nbinsy",&sh::Hist2D::getNBinsY)
    .add_property("underflowx",&sh::Hist2D::getUnderflowX)
    .add_property("overflowx",&sh::Hist2D::getOverflowX)
    .add_property("underflowy",&sh::Hist2D::getUnderflowY)
    .add_property("overflowy",&sh::Hist2D::getOverflowY)
    .add_property("minfilledx",&sh::Hist2D::getMinFilledX)
    .add_property("maxfilledx",&sh::Hist2D::getMaxFilledX)
    .add_property("minfilledy",&sh::Hist2D::getMinFilledY)
    .add_property("maxfilledy",&sh::Hist2D::getMaxFilledY)
    .add_property("xmin",&sh::Hist2D::getXMin)
    .add_property("xmax",&sh::Hist2D::getXMax)
    .add_property("ymin",&sh::Hist2D::getYMin)
    .add_property("ymax",&sh::Hist2D::getYMax)
    .add_property("meanx",&sh::Hist2D::getMeanX)
    .add_property("rmsx",&sh::Hist2D::getRMSX)
    .add_property("rms2x",&sh::Hist2D::getRMSSquaredX)
    .add_property("meany",&sh::Hist2D::getMeanY)
    .add_property("rmsy",&sh::Hist2D::getRMSY)
    .add_property("rms2y",&sh::Hist2D::getRMSSquaredY)
    .add_property("covariance",&sh::Hist2D::getCovariance)
    .add_property("covxy",&sh::Hist2D::getCovariance)
    .add_property("correlation",&sh::Hist2D::getCorrelation)
    .add_property("corxy",&sh::Hist2D::getCorrelation)
    ;

  //HistCounts::Counter:
  py::class_<sh::HistCounts::Counter>("Counter",py::no_init)
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
    .add_property("value",&sh::HistCounts::Counter::getValue)
    .add_property("error",&sh::HistCounts::Counter::getError)
    .add_property("errorsquared",&sh::HistCounts::Counter::getErrorSquared)
    .add_property("label",&shp::Counter_getLabel)
    .add_property("displaylabel",&shp::Counter_getDisplayLabel,&sh::HistCounts::Counter::setDisplayLabel)
    .add_property("comment",&shp::Counter_getComment,&sh::HistCounts::Counter::setComment)
    ;

  //HistCounts:
  py::class_<sh::HistCounts,boost::noncopyable,py::bases<sh::HistBase> >("HistCounts",py::init<>())
    .def(py::init<const std::string&>())
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
    .add_property("ncounters",&sh::HistCounts::nCounters)
    .add_property("maxcontent",&sh::HistCounts::getMaxContent)
    .add_property("counters",&shp::HistCounts_getCounters)
    ;

  //HistCollection:
  py::class_<sh::HistCollection,boost::noncopyable>("HistCollection",py::init<>())
    .def(py::init<const std::string&>())
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
    .def("removeAndManage",&sh::HistCollection::remove,py::return_value_policy<py::manage_new_object>())
    .def("saveToFile",&sh::HistCollection::saveToFile)
    .def("saveToFile",&shp::HistColl_saveToFile_1arg)
    .def("getKeys",&shp::HistCol_getKeys)
    .def("merge",&shp::HistCol_mergeCol)
    .def("merge",&shp::HistCol_mergeStr)
    .def("isSimilar",&sh::HistCollection::isSimilar)
    .add_property("keys",&shp::HistCol_getKeys)
    ;


}

