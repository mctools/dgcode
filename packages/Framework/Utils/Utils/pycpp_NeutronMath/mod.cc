#include "Core/Python.hh"
#include "Utils/NeutronMath.hh"

PYTHON_MODULE
{
  PYDEF("neutronWavelengthToEKin",Utils::neutronWavelengthToEKin);
  PYDEF("neutronEKinToWavelength",Utils::neutronEKinToWavelength);
  PYDEF("neutron_angstrom_to_meV",Utils::neutron_angstrom_to_meV);
  PYDEF("neutron_angstrom_to_eV",Utils::neutron_angstrom_to_eV);
  PYDEF("neutron_meV_to_angstrom",Utils::neutron_meV_to_angstrom);
  PYDEF("neutron_eV_to_angstrom",Utils:: neutron_eV_to_angstrom);
  PYDEF("neutron_angstrom_to_meters_per_second",Utils::neutron_angstrom_to_meters_per_second);
  PYDEF("neutron_meters_per_second_to_angstrom",Utils::neutron_meters_per_second_to_angstrom);
}
