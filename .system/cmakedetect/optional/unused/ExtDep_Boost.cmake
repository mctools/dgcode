set(HAS_Boost 0)
set(Boost_NO_BOOST_CMAKE ON)#to avoid getting a weird lib list with spurious optimized, debug and -shared parts
if (NOT DG_VERBOSE)
  set(Boost_FIND_QUIETLY ON)
else()
  set(Boost_DETAILED_FAILURE_MSG ON)
endif()
#To fail faster and get less weird errors, when boost is not installed on the
#system, we first look for just the headers:
find_package(Boost 1.41.0)
if(Boost_FOUND)
  find_package(Boost 1.41.0 COMPONENTS iostreams)
  if(Boost_FOUND)
    set(HAS_Boost 1)
    findpackage_liblist_to_flags("${Boost_LIBRARIES}" "${Boost_LIBRARY_DIRS}" ExtDep_Boost_LINK_FLAGS)
    set(ExtDep_Boost_COMPILE_FLAGS "-I${Boost_INCLUDE_DIRS}")#plural ..._DIRS but assume singular for now
    set(ExtDep_Boost_VERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
  endif()
endif()
