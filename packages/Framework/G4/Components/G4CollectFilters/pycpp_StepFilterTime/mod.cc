#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterTime.hh"

PYTHON_MODULE
{
  StepFilterPyExport::exportFilter<StepFilterTime>("StepFilterTime");
}
