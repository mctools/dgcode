#include "Core/Python.hh"
#include "XSectSpySteppingAction.hh"
#include "G4Launcher/Launcher.hh"

void spyG4XSect()
{
  auto launcher = G4Launcher::Launcher::getTheLauncher();
  if (!launcher) {
    printf("XSectSpy ERROR: Launcher not initialised!\n");
    exit(1);
  }
  launcher->setUserSteppingAction(new XSectSpySteppingAction(launcher->getPhysicsList()));
}

void spyOneG4XSect()
{
  auto launcher = G4Launcher::Launcher::getTheLauncher();
  if (!launcher) {
    printf("XSectSpy ERROR: Launcher not initialised!\n");
    exit(1);
  }
  launcher->setUserSteppingAction(new XSectSpySteppingAction(launcher->getPhysicsList(),1));
}

PYTHON_MODULE
{
  py::def("install",&spyG4XSect);
  py::def("installForOneFile",&spyOneG4XSect);
  py::def("lastWrittenFile",&XSectSpySteppingAction::lastWrittenFile);
  py::def("lastG4MaterialPrinted",&XSectSpySteppingAction::lastG4MaterialPrinted);

}
