#include <dgboost/python/operators.hpp>
namespace GriffDataRead {

  py::object py_get_elem_list( const Material*m )
  {
    py::list l;
    assert(m);
    const unsigned n = m->numberElements();
    for ( unsigned i = 0; i < n; ++i ) {
      auto elem = m->getElement(i);
      l.append( py::dg_ptr2pyobj_refexisting( elem ) );
    }
    return std::move(l);
  }
  py::object py_get_isotope_list( const Element*elem )
  {
    py::list l;
    assert(elem);
    const unsigned n = elem->numberIsotopes();
    for ( unsigned i = 0; i < n; ++i ) {
      l.append( py::dg_ptr2pyobj_refexisting( elem->getIsotope(i) ) );
    }
    return std::move(l);
  }

  //Due to lack of overloading, we add dump methods directly to the objects in python:
  void dump_mat(const Material*m) { dump(m); }
  void dump_elem(const Element*m) { dump(m); }
  void dump_iso(const Isotope*m) { dump(m); }
  intptr_t py_mat_id(const Material*ptr) { return intptr_t(ptr); }

  void pyexport_Material()
  {

    py::class_<GriffDataRead::Material,GriffDataRead::Material*,boost::noncopyable>("Material",py::no_init)
      .def("getName",&GriffDataRead::Material::getNameCStr)
      .def("density",&GriffDataRead::Material::density)
      .def("temperature",&GriffDataRead::Material::temperature)
      .def("pressure",&GriffDataRead::Material::pressure)
      .def("radiationLength",&GriffDataRead::Material::radiationLength)
      .def("nuclearInteractionLength",&GriffDataRead::Material::nuclearInteractionLength)
      .def("hasMeanExitationEnergy",&GriffDataRead::Material::hasMeanExitationEnergy)
      .def("meanExitationEnergy",&GriffDataRead::Material::meanExitationEnergy)
      .def("isSolid",&GriffDataRead::Material::isSolid)
      .def("isGas",&GriffDataRead::Material::isGas)
      .def("isLiquid",&GriffDataRead::Material::isLiquid)
      .def("isUndefinedState",&GriffDataRead::Material::isUndefinedState)
      .def("stateStr",&GriffDataRead::Material::stateCStr)
      .def("numberElements",&GriffDataRead::Material::numberElements)
      .def("elementFraction",&GriffDataRead::Material::elementFraction)
      .def("getElement",&GriffDataRead::Material::getElement,py::return_ptr())
      .add_property("elements", &GriffDataRead::py_get_elem_list)

      .def("dump",&GriffDataRead::dump_mat)
      .def("transient_id",&GriffDataRead::py_mat_id)//temporary workaround...
      ;

    py::class_<GriffDataRead::Element,boost::noncopyable>("Element",py::no_init)
      .def("getName",&GriffDataRead::Element::getNameCStr)
      .def("getSymbol",&GriffDataRead::Element::getSymbolCStr)
      .def("Z",&GriffDataRead::Element::Z)
      .def("N",&GriffDataRead::Element::N)
      .def("A",&GriffDataRead::Element::A)
      .def("naturalAbundances",&GriffDataRead::Element::naturalAbundances)
      .def("numberIsotopes",&GriffDataRead::Element::numberIsotopes)
      .def("isotopeRelativeAbundance",&GriffDataRead::Element::isotopeRelativeAbundance)
      .def("getIsotope",&GriffDataRead::Element::getIsotope,py::return_ptr())
      .add_property("isotopes", &GriffDataRead::py_get_isotope_list)
      .def("dump",&GriffDataRead::dump_elem)
      ;

    py::class_<GriffDataRead::Isotope,boost::noncopyable>("Isotope",py::no_init)
      .def("getName",&GriffDataRead::Isotope::getNameCStr)
      .def("Z",&GriffDataRead::Isotope::Z)
      .def("N",&GriffDataRead::Isotope::N)
      .def("A",&GriffDataRead::Isotope::A)
      .def("m",&GriffDataRead::Isotope::m)
      .def("dump",&GriffDataRead::dump_iso)
      ;

  }

}
