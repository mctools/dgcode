#include "G4Interfaces/StepFilterPyExport.hh"
#include "G4CollectFilters/StepFilterVolume.hh"

PYTHON_MODULE
{
  StepFilterPyExport::exportFilter<StepFilterVolume>("StepFilterVolume");
}
