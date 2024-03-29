set(HAS_ZLib 0)
if (NOT SBLD_VERBOSE)
  set(ZLIB_FIND_QUIETLY ON)
endif()
FIND_PACKAGE(ZLIB)
if (ZLIB_FOUND)
    set(HAS_ZLib 1)
    if (NOT "x${ZLIB_INCLUDE_DIRS}" STREQUAL "x/usr/include")
      set(ExtDep_ZLib_COMPILE_FLAGS "${ExtDep_ZLib_COMPILE_FLAGS} -I${ZLIB_INCLUDE_DIRS} -isystem${ZLIB_INCLUDE_DIRS}")
    else()
      set(ExtDep_ZLib_COMPILE_FLAGS "${ExtDep_ZLib_COMPILE_FLAGS}")
    endif()
    #get_filename_component(ZLIB_LIBRARIES  "${ZLIB_LIBRARIES}" REALPATH)
    set(ExtDep_ZLib_LINK_FLAGS "${ZLIB_LIBRARIES}")
    set(ExtDep_ZLib_VERSION "${ZLIB_VERSION_STRING}")
endif()
