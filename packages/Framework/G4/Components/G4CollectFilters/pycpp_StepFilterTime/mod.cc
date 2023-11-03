#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterTime.hh"

PYTHON_MODULE3
{
  StepFilterPyExport::exportFilter<StepFilterTime>(mod, "StepFilterTime");
}
