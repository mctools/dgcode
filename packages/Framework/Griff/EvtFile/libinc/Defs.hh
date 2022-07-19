#ifndef EvtFile_Defs_hh
#define EvtFile_Defs_hh

#include "Core/Types.hh"
#include <string>

namespace EvtFile {

  typedef uint32_t index_type;
  static const index_type INDEX_MAX = UINT32_MAX;
  typedef uint16_t subsectid_type;

  typedef std::string str_type;

}

#endif
