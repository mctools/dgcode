#include "Core/Python.hh"
#include <stdexcept>

namespace {
  void raw_dgbuild_pyInit()
  {
    if (::pyextra::isPyInit())
      throw std::runtime_error("Attempt at initialising Python interpreter twice detected");
    Py_Initialize();
  }
}

bool py::isPyInit()
{
  return Py_IsInitialized();
}

void py::pyInit(const char * argv0)
{
  raw_dgbuild_pyInit();
  //Always put at least a dummy entry for sys.argv[0], since some python modules
  //will assume this is always present (for instance the matplotlib tkinter
  //backend with python 3.7):
  py::list pysysargv;
  pysysargv.append(std::string(argv0?argv0:"dummyargv0"));
  py::pyimport("sys").attr("argv")=pysysargv;
}

void py::pyInit(int argc, char** argv)
{
  if (argc==0) {
    pyInit();
    return;
  }
  raw_dgbuild_pyInit();
  py::list pysysargv;
  for (int i = 0; i < argc; ++i)
    pysysargv.append(std::string(argv[i]));
  py::pyimport("sys").attr("argv")=pysysargv;
}


void pyextra::pyInit(const char * argv0)
{
  raw_dgbuild_pyInit();
  //Always put at least a dummy entry for sys.argv[0], since some python modules
  //will assume this is always present (for instance the matplotlib tkinter
  //backend with python 3.7):
  py::list pysysargv;
  pysysargv.append(std::string(argv0?argv0:"dummyargv0"));
  py::pyimport("sys").attr("argv")=pysysargv;
}

void pyextra::pyInit(int argc, char** argv)
{
  if (argc==0) {
    pyInit();
    return;
  }
  raw_dgbuild_pyInit();
  py::list pysysargv;
  for (int i = 0; i < argc; ++i)
    pysysargv.append(std::string(argv[i]));
  py::pyimport("sys").attr("argv")=pysysargv;
}

bool pyextra::isPyInit()
{
  return Py_IsInitialized();
}

void pyextra::ensurePyInit()
{
  if (!isPyInit())
    pyInit();
}

void py::ensurePyInit() {
  if (!isPyInit())
    pyInit();
}
