#include "Core/Python.hh"
#include "Utils/Url2Local.hh"

std::string Utils::url2local(std::string input_filename, std::string cachedir)
{
  py::ensurePyInit();
  py::object mod = py::pyimport("Utils.dl_url_to_local_cache");
  auto res = mod.attr("dl_with_cache")(input_filename,cachedir);
  return py::extract<std::string>(res);
}
