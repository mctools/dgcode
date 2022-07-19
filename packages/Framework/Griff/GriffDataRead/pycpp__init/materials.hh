#include <dgboost/python/operators.hpp>
namespace GriffDataRead {

  struct py_elem_iter {
    static py_elem_iter create(const Material*m) { return py_elem_iter(m); }
    py_elem_iter(const Material*m) : m_i(0), m_n(m->numberElements()), m_mat(m) {}
    py_elem_iter* thisptr() { return this; }
    const Element* next() {
      if (m_i==m_n) {
        PyErr_SetString(PyExc_StopIteration, "No more data.");
        boost::python::throw_error_already_set();
      }
      assert(m_i!=m_n);
      return m_mat->getElement(m_i++);
    }
    unsigned m_i;
    unsigned m_n;
    const Material* m_mat;
    static void pyexport()
    {
      py::class_<GriffDataRead::py_elem_iter>("py_elem_iter",py::no_init)
#if PY_MAJOR_VERSION<3
        .def("next",&GriffDataRead::py_elem_iter::next,py::return_ptr())
#else
        .def("__next__",&GriffDataRead::py_elem_iter::next,py::return_ptr())
#endif
        .def("__iter__",&GriffDataRead::py_elem_iter::thisptr,py::return_ptr())
        ;
    }
  };
  struct py_isotope_iter {
    static py_isotope_iter create(const Element*e) { return py_isotope_iter(e); }
    py_isotope_iter(const Element*e) : m_i(0), m_n(e->numberIsotopes()), m_elem(e) {}
    py_isotope_iter* thisptr() { return this; }
    const Isotope* next() {
      if (m_i==m_n) {
        PyErr_SetString(PyExc_StopIteration, "No more data.");
        boost::python::throw_error_already_set();
      }
      assert(m_i!=m_n);
      return m_elem->getIsotope(m_i++);
    }
    unsigned m_i;
    unsigned m_n;
    const Element* m_elem;
    static void pyexport()
    {
      py::class_<GriffDataRead::py_isotope_iter>("py_isotope_iter",py::no_init)
#if PY_MAJOR_VERSION<3
        .def("next",&GriffDataRead::py_isotope_iter::next,py::return_ptr())
#else
        .def("__next__",&GriffDataRead::py_isotope_iter::next,py::return_ptr())
#endif
        .def("__iter__",&GriffDataRead::py_isotope_iter::thisptr,py::return_ptr())
        ;
    }
  };

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
      .add_property("elements", &GriffDataRead::py_elem_iter::create)
      .def("dump",&GriffDataRead::dump_mat)
      .def("transient_id",&GriffDataRead::py_mat_id)//temporary workaround...
      ;

    py_elem_iter::pyexport();

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
      .add_property("isotopes", &GriffDataRead::py_isotope_iter::create)
      .def("dump",&GriffDataRead::dump_elem)
      ;

    py_isotope_iter::pyexport();

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
