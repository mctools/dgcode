#include "Core/Python.hh"
#include "G4DataCollect/G4DataCollect.hh"

PYTHON_MODULE
{
#if defined(DGCODE_USEPYBIND11)
  PYDEF("installHooks",&G4DataCollect::installHooks,
        "Installs hooks necessary to record the output of the Geant4 simulation.",
        py::arg("outputFile"), py::arg("mode")="FULL"
        );
#else
  PYDEF("installHooks",&G4DataCollect::installHooks,
          ( py::arg("outputFile"), py::arg("mode")="FULL" ),
          "Installs hooks necessary to record the output of the Geant4 simulation.");
#endif
  PYDEF("finish",&G4DataCollect::finish,"Uninstall hooks and close output file.");

  PYDEF("setMetaData",&G4DataCollect::setMetaData);
  PYDEF("setUserData",&G4DataCollect::setUserData);

}
