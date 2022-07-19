#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterPrimary.hh"

PYTHON_MODULE
{
  StepFilterPyExport::exportFilter<StepFilterPrimary>("StepFilterPrimary");
}
