function( detect_system_boostpython resvar_found resvar_cflags_list resvar_linkflags_list resvar_boostincdir )
  set( all_ok OFF )
  set( ${resvar_found} OFF PARENT_SCOPE )
  if ( DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/include/boost/python.hpp" )
    file( GLOB tmp "$ENV{CONDA_PREFIX}/lib/cmake/Boost-*/BoostConfig.cmake" )
    list( LENGTH tmp tmplen )
    if ( tmplen EQUAL 1 )
      list( POP_FRONT tmp Boost_DIR )
      cmake_path( GET Boost_DIR PARENT_PATH Boost_DIR )
      cmake_path( ABSOLUTE_PATH Boost_DIR NORMALIZE )
      set( Boost_USE_STATIC_RUNTIME OFF )
      set( tmp_findpkgargs Boost 1.78 REQUIRED NO_MODULE NO_DEFAULT_PATH COMPONENTS python )
      find_package( ${tmp_findpkgargs} )
      if ( Boost_FOUND )
        extract_extdep_flags( CXX "${tmp_findpkgargs}" "Boost::python"
          "-DBoost_DIR=${Boost_DIR};-DBoost_USE_STATIC_RUNTIME=OFF"
          boostpy_cxx_cflags boostpy_cxx_linkflags )
        #message("GOT boostpy_cxx_cflags: ${boostpy_cxx_cflags}")
        #message("GOT boostpy_cxx_linkflags: ${boostpy_cxx_linkflags}")
        set( all_ok ON )
        message( "-- Will use conda-provided boost-python!" )
        set( ${resvar_found} ON PARENT_SCOPE )
        set( ${resvar_cflags_list} "${boostpy_cxx_cflags}" PARENT_SCOPE )
        set( ${resvar_linkflags_list} "${boostpy_cxx_linkflags}" PARENT_SCOPE )
        set( ${resvar_boostincdir} "$ENV{CONDA_PREFIX}/include/boost" PARENT_SCOPE )
      endif()
    endif()
  endif()
  if ( NOT all_ok )
    message( "-- Did not find suitable conda-provided boost-python." )
  endif()
endfunction()

if ( NOT strategy_syspyboost STREQUAL "never" )
  detect_system_boostpython( SYSTEM_BOOSTPYTHON_FOUND SYSTEM_BOOSTPYTHON_CFLAGS_LIST SYSTEM_BOOSTPYTHON_LINKFLAGS_LIST SYSTEM_BOOSTPYTHON_INCDIR )
else()
  message( "-- Skipping search for conda-provided boost-python due to value of DGCODE_USECONDABOOSTPYTHON." )
  set( SYSTEM_BOOSTPYTHON_FOUND OFF )
endif()

if ( SYSTEM_BOOSTPYTHON_FOUND )
  message("-- boost-python compilation flags : ${SYSTEM_BOOSTPYTHON_CFLAGS_LIST} " )
  message("-- boost-python link flags : ${SYSTEM_BOOSTPYTHON_LINKFLAGS_LIST} " )
  message("-- boost-python headers : ${SYSTEM_BOOSTPYTHON_INCDIR} " )
else()
  if ( strategy_syspyboost STREQUAL "always" )
    message( "-- DGCODE_USECONDABOOSTPYTHON==always but could not find conda-provided boost-python." )
  endif()
  set( SYSTEM_BOOSTPYTHON_CFLAGS_LIST "" )
  set( SYSTEM_BOOSTPYTHON_LINKFLAGS_LIST "" )
  set( SYSTEM_BOOSTPYTHON_INCDIR "" )
endif()

