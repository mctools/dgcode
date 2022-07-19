#include "Core/Python.hh"
#include "G4DataCollect/G4DataCollect.hh"

PYTHON_MODULE
{
  py::def("installHooks",&G4DataCollect::installHooks,
          ( py::arg("outputFile"), py::arg("mode")="FULL" ),
          "install hooks necessary to record the output of the Geant4 simulation");

  py::def("finish",&G4DataCollect::finish,"de-install hooks and close output file");

  py::def("setMetaData",&G4DataCollect::setMetaData);
  py::def("setUserData",&G4DataCollect::setUserData);

}
