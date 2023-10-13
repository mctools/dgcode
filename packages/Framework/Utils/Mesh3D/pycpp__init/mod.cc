#include "Core/Python.hh"
#include "Mesh/Mesh.hh"
#include "Utils/NumpyUtils.hh"

namespace {

#ifdef DGCODE_USEPYBIND11
  using PyNumpyArrayDbl = py::array_t<double,py::array::c_style>;
#endif

class py_Mesh3D {
public:
  py_Mesh3D(const std::string& filename)
  {
    Mesh::Mesh<3> mesh(filename);
    m_name = mesh.name();
    m_comments = mesh.comments();
    m_cellunits = mesh.cellunits();

#ifdef DGCODE_USEPYBIND11
    for ( auto& e : mesh.statMap() )
      m_stats[py::str(e.first)] = e.second;
#else
    auto it = mesh.statMap().begin();
    auto itE = mesh.statMap().end();
    for (;it!=itE;++it)
      m_stats[it->first] = it->second;
#endif
    for (int i = 0; i < 3; ++i) {
      assert( mesh.filler().nCells(i) >= 0 );
      m_cells_n[i] = static_cast<std::size_t>(mesh.filler().nCells(i));
      m_cells_py.append(py::make_tuple(mesh.filler().nCells(i),
                                       mesh.filler().cellLower(i),
                                       mesh.filler().cellUpper(i)));
    }
    auto & datavect = mesh.filler().data();
    //init m_data as numpy array and access raw buffer:
#ifdef DGCODE_USEPYBIND11
    m_data = PyNumpyArrayDbl( {mesh.filler().nCells(0),mesh.filler().nCells(1),mesh.filler().nCells(2)} );
    std::size_t ntot = static_cast<std::size_t>( m_data.size() );
    double * buf = m_data.mutable_data();
#else
    auto numpy = py::pyimport("numpy");
    auto shape = py::make_tuple(mesh.filler().nCells(0),
                                mesh.filler().nCells(1),
                                mesh.filler().nCells(2));
    m_data = numpy.attr("zeros")(shape,"float64");
    double* buf;
    std::size_t ntot;
    NumpyUtils::decodeBuffer(m_data,buf,ntot);
#endif
    assert( ntot == datavect.size() );
    assert(buf);
    size_t iblock = 0;
    size_t nblocks = datavect.blockCount();
    while ( (iblock = datavect.nextAllocBlock(iblock)) < nblocks) {
      auto blockdata = datavect.block(iblock);
      assert(blockdata);
      std::memcpy(&buf[datavect.block_size * iblock],&((*blockdata)[0]),sizeof(double)*blockdata->size());
      ++iblock;
    }
  }
  const char * getName() const { return m_name.c_str(); }
  const char * getComments() const { return m_comments.c_str(); }
  const char * getCellUnits() const { return m_cellunits.c_str(); }
  py::object getData() const { return m_data; }
  py::list getCellInfo_py() const { return m_cells_py; }

  py::dict getStats() const { return m_stats; }

  void print_summary() const {
    printf("Mesh3D:\n");
    printf("  Name       : %s\n",(m_name.empty()?"<none>":m_name.c_str()));
    printf("  Comments   : %s\n",(m_comments.empty()?"<none>":m_comments.c_str()));
    printf("  Cell units : %s\n",(m_cellunits.empty()?"<none>":m_cellunits.c_str()));
#if 0
    //Would be this simple if python 2.6 didn't produce slightly different
    //output of floats inside containers, breaking unit tests:
    std::string tmp = py::extract<std::string>(py::str(m_cells_py));
    printf("  Cells      : %s\n",tmp.c_str());
    tmp = py::extract<std::string>(py::str(m_stats));
    printf("  Stats      : %s\n",tmp.c_str());
#else
    //But we do it the hard way instead:
#  ifdef DGCODE_USEPYBIND11
    std::string tmp[9];
    for (int i=0;i<9;++i)
      tmp[i] = py::cast<std::string>(py::str(m_cells_py[i/3].attr("__getitem__")(i%3)));//what a mess...
    //      tmp[i] = py::cast<std::string>(py::str(std::to_string(py::cast<double>(m_cells_py[i/3].attr("__getitem__")(i%3)))));//what a mess...
    printf("  Cells      : [(%s, %s, %s), (%s, %s, %s), (%s, %s, %s)]\n",
           tmp[0].c_str(),tmp[1].c_str(),tmp[2].c_str(),
           tmp[3].c_str(),tmp[4].c_str(),tmp[5].c_str(),
           tmp[6].c_str(),tmp[7].c_str(),tmp[8].c_str());
    py::list keys;
    for (auto& e : m_stats )
      keys.append( e.first );

    //py::list keys = m_stats.attr("keys")();
    keys.attr("sort")();
    printf("  Stats      : {");
    for (py::size_t i = 0; i<py::len(keys); ++i) {
      tmp[0] = py::extract<std::string>(keys[i]);
      py::object o = m_stats[keys[i]];
      tmp[1] = py::extract<std::string>(py::str(o));
      printf("'%s': %s%s",
             tmp[0].c_str(),tmp[1].c_str(),
             (i+1==py::len(keys)?"}\n":", "));
    }
#  else
    std::string tmp[9];
    for (int i=0;i<9;++i)
      tmp[i] = py::extract<std::string>(py::str(m_cells_py[i/3][i%3]));
    printf("  Cells      : [(%s, %s, %s), (%s, %s, %s), (%s, %s, %s)]\n",
           tmp[0].c_str(),tmp[1].c_str(),tmp[2].c_str(),
           tmp[3].c_str(),tmp[4].c_str(),tmp[5].c_str(),
           tmp[6].c_str(),tmp[7].c_str(),tmp[8].c_str());
    py::list keys = m_stats.keys();
    keys.attr("sort")();
    printf("  Stats      : {");
    for (int i = 0; i<py::len(keys); ++i) {
      tmp[0] = py::extract<std::string>(keys[i]);
      py::object o = m_stats[keys[i]];
      tmp[1] = py::extract<std::string>(py::str(o));
      printf("'%s': %s%s",
             tmp[0].c_str(),tmp[1].c_str(),
             (i+1==py::len(keys)?"}\n":", "));
    }
#  endif
#endif
  }

  void print_cells(bool include_empty) const {
#ifdef DGCODE_USEPYBIND11
    std::size_t ntot = static_cast<std::size_t>( m_data.size() );
    const double * buf = m_data.data();
    assert(buf);
    auto nx = m_cells_n[0];
    auto ny = m_cells_n[1];
    auto nz = m_cells_n[2];
    assert(std::size_t(nx*ny*nz)==ntot);
    for (std::int64_t ix = 0; ix < nx; ++ix)
      for (std::int64_t iy = 0; iy < ny; ++iy)
        for (std::int64_t iz = 0; iz < nz; ++iz) {
          double v = buf[ix*ny*nz+iy*nz+iz];
          if (v||include_empty)
            printf("  cell[%li,%li,%li] = %g\n",ix,iy,iz,v);
        }
#else
    double* buf;
    std::size_t ntot;
    NumpyUtils::decodeBuffer(m_data,buf,ntot);
    assert(buf);
    auto nx = m_cells_n[0];
    auto ny = m_cells_n[1];
    auto nz = m_cells_n[2];
    assert(std::size_t(nx*ny*nz)==ntot);
    for (std::int64_t ix = 0; ix < nx; ++ix)
      for (std::int64_t iy = 0; iy < ny; ++iy)
        for (std::int64_t iz = 0; iz < nz; ++iz) {
          double v = buf[ix*ny*nz+iy*nz+iz];
          if (v||include_empty)
            printf("  cell[%li,%li,%li] = %g\n",ix,iy,iz,v);
        }
#endif
  }
private:
  std::string m_name;
  std::string m_comments;
  std::string m_cellunits;
  py::dict m_stats;
#ifdef DGCODE_USEPYBIND11
  PyNumpyArrayDbl m_data;
#else
  py::object m_data;
#endif
  py::list m_cells_py;
  std::int64_t m_cells_n[3];
};

void py_Mesh3D_merge_files(std::string output_file, py::list input_files) {
  py::ssize_t n = py::len(input_files);
  if (n<2) {
    PyErr_SetString(PyExc_ValueError, "List of files to merge must be at least length 2");
    throw py::error_already_set();
  }
  std::string tmp;
  tmp = py::extract<std::string>(input_files[0]);
  Mesh::Mesh<3> mesh(tmp);
  for (py::ssize_t i = 1; i < n; ++i) {
    tmp = py::extract<std::string>(input_files[i]);
    mesh.merge(tmp);
  }
  mesh.saveToFile(output_file);
}
}

PYTHON_MODULE
{

#ifdef DGCODE_USEPYBIND11
  py::class_<py_Mesh3D>(m,"Mesh3D")
    .def( py::init<std::string>() )
#else
  py::class_<py_Mesh3D,boost::noncopyable>( "Mesh3D",
                                            py::init<std::string>() )
#endif
    .PYADDREADONLYPROPERTY("name",&py_Mesh3D::getName)
    .PYADDREADONLYPROPERTY("comments",&py_Mesh3D::getComments)
    .PYADDREADONLYPROPERTY("cellunits",&py_Mesh3D::getCellUnits)
    .PYADDREADONLYPROPERTY("data",&py_Mesh3D::getData)
    .PYADDREADONLYPROPERTY("cellinfo",&py_Mesh3D::getCellInfo_py)
    .PYADDREADONLYPROPERTY("stats",&py_Mesh3D::getStats)
    .def("dump_cells",&py_Mesh3D::print_cells)
    .def("print_summary",&py_Mesh3D::print_summary)
    ;

  PYDEF("merge_files",&py_Mesh3D_merge_files);

}
