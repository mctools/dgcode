#include "Core/Python.hh"
#include "Utils/MaxwellDist.hh"

PYTHON_MODULE
{
  PYDEF("maxwellDist",Utils::maxwellDist);
  PYDEF("maxwellDistCommulative",Utils::maxwellDistCommulative);
  PYDEF("maxwellShoot",Utils::maxwellShoot);
  PYDEF("maxwellParFromMean",Utils::maxwellParFromMean);
  PYDEF("maxwellParFromPeak",Utils::maxwellParFromPeak);
  PYDEF("maxwellMean",Utils::maxwellMean);
  PYDEF("maxwellPeak",Utils::maxwellPeak);
  PYDEF("maxwellRMS",Utils::maxwellRMS);
  PYDEF("thermalMaxwellPar",Utils::thermalMaxwellPar);
  PYDEF("shootThermalSpeed",Utils::shootThermalSpeed);
  PYDEF("shootThermalEKin",Utils::shootThermalEKin);
  PYDEF("shootThermalWavelength",Utils::shootThermalWavelength);
  PYDEF("thermalNeutronMaxwellPar",Utils::thermalNeutronMaxwellPar);
  PYDEF("shootThermalNeutronSpeed",Utils::shootThermalNeutronSpeed);
  PYDEF("shootThermalNeutronEKin",Utils::shootThermalNeutronEKin);
  PYDEF("shootThermalNeutronWavelength",Utils::shootThermalNeutronWavelength);
  PYDEF("thermalEnergyDistPeak",Utils::thermalEnergyDistPeak);
  PYDEF("thermalEnergyDistMean",Utils::thermalEnergyDistMean);
  PYDEF("thermalEnergyDistMedian",Utils::thermalEnergyDistMedian);
  PYDEF("thermalEnergyDist",Utils::thermalEnergyDist);
  PYDEF("thermalEnergyDistCommulative",Utils::thermalEnergyDistCommulative);
}
