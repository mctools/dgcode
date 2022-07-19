#include "Core/Python.hh"
#include "Utils/MaxwellDist.hh"

PYTHON_MODULE
{
  py::def("maxwellDist",Utils::maxwellDist);
  py::def("maxwellDistCommulative",Utils::maxwellDistCommulative);
  py::def("maxwellShoot",Utils::maxwellShoot);
  py::def("maxwellParFromMean",Utils::maxwellParFromMean);
  py::def("maxwellParFromPeak",Utils::maxwellParFromPeak);
  py::def("maxwellMean",Utils::maxwellMean);
  py::def("maxwellPeak",Utils::maxwellPeak);
  py::def("maxwellRMS",Utils::maxwellRMS);
  py::def("thermalMaxwellPar",Utils::thermalMaxwellPar);
  py::def("shootThermalSpeed",Utils::shootThermalSpeed);
  py::def("shootThermalEKin",Utils::shootThermalEKin);
  py::def("shootThermalWavelength",Utils::shootThermalWavelength);
  py::def("thermalNeutronMaxwellPar",Utils::thermalNeutronMaxwellPar);
  py::def("shootThermalNeutronSpeed",Utils::shootThermalNeutronSpeed);
  py::def("shootThermalNeutronEKin",Utils::shootThermalNeutronEKin);
  py::def("shootThermalNeutronWavelength",Utils::shootThermalNeutronWavelength);
  py::def("thermalEnergyDistPeak",Utils::thermalEnergyDistPeak);
  py::def("thermalEnergyDistMean",Utils::thermalEnergyDistMean);
  py::def("thermalEnergyDistMedian",Utils::thermalEnergyDistMedian);
  py::def("thermalEnergyDist",Utils::thermalEnergyDist);
  py::def("thermalEnergyDistCommulative",Utils::thermalEnergyDistCommulative);
}
