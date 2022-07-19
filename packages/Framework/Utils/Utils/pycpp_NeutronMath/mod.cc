#include "Core/Python.hh"
#include "Utils/NeutronMath.hh"

PYTHON_MODULE
{
  py::def("neutronWavelengthToEKin",Utils::neutronWavelengthToEKin);
  py::def("neutronEKinToWavelength",Utils::neutronEKinToWavelength);
  py::def("neutron_angstrom_to_meV",Utils::neutron_angstrom_to_meV);
  py::def("neutron_angstrom_to_eV",Utils::neutron_angstrom_to_eV);
  py::def("neutron_meV_to_angstrom",Utils::neutron_meV_to_angstrom);
  py::def("neutron_eV_to_angstrom",Utils:: neutron_eV_to_angstrom);
  py::def("neutron_angstrom_to_meters_per_second",Utils::neutron_angstrom_to_meters_per_second);
  py::def("neutron_meters_per_second_to_angstrom",Utils::neutron_meters_per_second_to_angstrom);
}
