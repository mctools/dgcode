#include "Core/Python.hh"
#include "GriffFormat/DumpFile.hh"

bool pydump_dumpFileInfo_1args( const char* filename ) { return GriffFormat::dumpFileInfo(filename); }
bool pydump_dumpFileInfo_2args( const char* filename, bool b ) { return GriffFormat::dumpFileInfo(filename,b); }

PYTHON_MODULE
{
  py::def("dumpFileInfo",&GriffFormat::dumpFileInfo,
          ( py::arg("filename") ),
          "dump content of G4 event file");

  py::def("dumpFileInfo",&pydump_dumpFileInfo_1args,
          ( py::arg("filename") ),
          "dump content of G4 event file");

  py::def("dumpFileInfo",&pydump_dumpFileInfo_2args,
          ( py::arg("filename") ),
          "dump content of G4 event file");

  py::def("dumpFileEventDBSection",&GriffFormat::dumpFileEventDBSection,
          ( py::arg("filename"), py::arg("evtIndex") ),
          "binary dump to stdout of the content of the database section of evt at evtIndex position in Griff file");

  py::def("dumpFileEventBriefDataSection",&GriffFormat::dumpFileEventBriefDataSection,
          ( py::arg("filename"), py::arg("evtIndex") ),
          "binary dump to stdout of the content of the brief data section of evt at evtIndex position in Griff file");

  py::def("dumpFileEventFullDataSection",&GriffFormat::dumpFileEventFullDataSection,
          ( py::arg("filename"), py::arg("evtIndex") ),
           "binary dump to stdout of the content of the full data section of evt at evtIndex position in Griff file");

}
