#include "Core/Python.hh"
#include "GriffFormat/DumpFile.hh"

#ifndef DGCODE_USEPYBIND11
namespace {
  bool pydump_dumpFileInfo_1args( const char* filename ) { return GriffFormat::dumpFileInfo(filename); }
  bool pydump_dumpFileInfo_2args( const char* filename, bool b ) { return GriffFormat::dumpFileInfo(filename,b); }
}
#endif

PYTHON_MODULE
{
#ifdef DGCODE_USEPYBIND11
  PYDEF("dumpFileInfo",&GriffFormat::dumpFileInfo,
        "dump content of G4 event file",
        py::arg("filename"),
        py::arg("brief")=false,
        py::arg("show_uncompressed_sizes")=false
        );

  PYDEF("dumpFileEventDBSection",&GriffFormat::dumpFileEventDBSection,
        "binary dump to stdout of the content of the database section of evt at evtIndex position in Griff file",
        py::arg("filename"), py::arg("evtIndex")
        );

  PYDEF("dumpFileEventBriefDataSection",&GriffFormat::dumpFileEventBriefDataSection,
        "binary dump to stdout of the content of the brief data section of evt at evtIndex position in Griff file",
        py::arg("filename"), py::arg("evtIndex")
        );

  PYDEF("dumpFileEventFullDataSection",&GriffFormat::dumpFileEventFullDataSection,
        "binary dump to stdout of the content of the full data section of evt at evtIndex position in Griff file",
        py::arg("filename"), py::arg("evtIndex")
        );
#else
  PYDEF("dumpFileInfo",&GriffFormat::dumpFileInfo,
        ( py::arg("filename") ),
        "dump content of G4 event file");

  PYDEF("dumpFileInfo",&pydump_dumpFileInfo_1args,
          ( py::arg("filename") ),
          "dump content of G4 event file");

  PYDEF("dumpFileInfo",&pydump_dumpFileInfo_2args,
          ( py::arg("filename") ),
          "dump content of G4 event file");

  PYDEF("dumpFileEventDBSection",&GriffFormat::dumpFileEventDBSection,
          ( py::arg("filename"), py::arg("evtIndex") ),
          "binary dump to stdout of the content of the database section of evt at evtIndex position in Griff file");

  PYDEF("dumpFileEventBriefDataSection",&GriffFormat::dumpFileEventBriefDataSection,
          ( py::arg("filename"), py::arg("evtIndex") ),
          "binary dump to stdout of the content of the brief data section of evt at evtIndex position in Griff file");

  PYDEF("dumpFileEventFullDataSection",&GriffFormat::dumpFileEventFullDataSection,
          ( py::arg("filename"), py::arg("evtIndex") ),
           "binary dump to stdout of the content of the full data section of evt at evtIndex position in Griff file");
#endif
}
