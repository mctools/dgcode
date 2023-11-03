#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterPrimary.hh"

PYTHON_MODULE3
{
  StepFilterPyExport::exportFilter<StepFilterPrimary>(mod, "StepFilterPrimary");
}
