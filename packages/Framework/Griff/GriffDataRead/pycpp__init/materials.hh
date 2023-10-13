#ifdef DGCODE_USEPYBIND11
namespace {
  template < typename T>
  struct BlankDeleter
  {
    void operator()(T *) const {}
  };
}
#else
#  include <dgboost/python/operators.hpp>
#endif

namespace GriffDataRead {

  py::object py_get_elem_list( const Material*m )
  {
    py::list l;
    assert(m);
    const unsigned n = m->numberElements();
    for ( unsigned i = 0; i < n; ++i ) {
      auto elem = m->getElement(i);
#ifdef DGCODE_USEPYBIND11
      l.append( py::cast( elem ) );
#else
      l.append( py::dg_ptr2pyobj_refexisting( elem ) );
#endif
    }
    return std::move(l);
  }
  py::object py_get_isotope_list( const Element*elem )
  {
    py::list l;
    assert(elem);
    const unsigned n = elem->numberIsotopes();
    for ( unsigned i = 0; i < n; ++i ) {
#ifdef DGCODE_USEPYBIND11
      l.append( py::cast( elem->getIsotope(i) ) );
#else
      l.append( py::dg_ptr2pyobj_refexisting( elem->getIsotope(i) ) );
#endif
    }
    return std::move(l);
  }

  //Due to lack of overloading, we add dump methods directly to the objects in python:
  void dump_mat(const Material*m) { dump(m); }
  void dump_elem(const Element*m) { dump(m); }
  void dump_iso(const Isotope*m) { dump(m); }
  intptr_t py_mat_id(const Material*ptr) { return intptr_t(ptr); }

#ifdef DGCODE_USEPYBIND11
  void pyexport_Material( py::module_ themod )
#else
  void pyexport_Material()
#endif
  {

#ifdef DGCODE_USEPYBIND11
    py::class_<Material,std::unique_ptr<Material, BlankDeleter<Material>>>(themod, "Material")
#else
    py::class_<Material,Material*,boost::noncopyable>("Material",py::no_init)
#endif
      .def("getName",&Material::getNameCStr)
      .def("density",&Material::density)
      .def("temperature",&Material::temperature)
      .def("pressure",&Material::pressure)
      .def("radiationLength",&Material::radiationLength)
      .def("nuclearInteractionLength",&Material::nuclearInteractionLength)
      .def("hasMeanExitationEnergy",&Material::hasMeanExitationEnergy)
      .def("meanExitationEnergy",&Material::meanExitationEnergy)
      .def("isSolid",&Material::isSolid)
      .def("isGas",&Material::isGas)
      .def("isLiquid",&Material::isLiquid)
      .def("isUndefinedState",&Material::isUndefinedState)
      .def("stateStr",&Material::stateCStr)
      .def("numberElements",&Material::numberElements)
      .def("elementFraction",&Material::elementFraction)
      .def("getElement",&Material::getElement,py::return_ptr())
      .PYADDREADONLYPROPERTY("elements", &py_get_elem_list)
      .def("dump",&dump_mat)
      .def("transient_id",&py_mat_id)//temporary workaround...
      ;

#ifdef DGCODE_USEPYBIND11
    py::class_<Element,std::unique_ptr<Element, BlankDeleter<Element>>>(themod, "Element")
#else
    py::class_<Element,boost::noncopyable>("Element",py::no_init)
#endif
      .def("getName",&Element::getNameCStr)
      .def("getSymbol",&Element::getSymbolCStr)
      .def("Z",&Element::Z)
      .def("N",&Element::N)
      .def("A",&Element::A)
      .def("naturalAbundances",&Element::naturalAbundances)
      .def("numberIsotopes",&Element::numberIsotopes)
      .def("isotopeRelativeAbundance",&Element::isotopeRelativeAbundance)
      .def("getIsotope",&Element::getIsotope,py::return_ptr())
      .PYADDREADONLYPROPERTY("isotopes", &py_get_isotope_list)
      .def("dump",&dump_elem)
      ;

#ifdef DGCODE_USEPYBIND11
    py::class_<Isotope,std::unique_ptr<Isotope, BlankDeleter<Isotope>>>(themod, "Isotope")
#else
    py::class_<Isotope,boost::noncopyable>("Isotope",py::no_init)
#endif
      .def("getName",&Isotope::getNameCStr)
      .def("Z",&Isotope::Z)
      .def("N",&Isotope::N)
      .def("A",&Isotope::A)
      .def("m",&Isotope::m)
      .def("dump",&dump_iso)
      ;

  }

}
