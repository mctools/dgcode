function( detect_system_pybind11
    resvar_embed_cflags_list
    resvar_embed_linkflags_list
    resvar_module_cflags_list
    resvar_module_linkflags_list
    resvar_version
    )
  set( cmd "${PYTHON_EXECUTABLE}" -mpybind11 --cmakedir )
  if ( DG_VERBOSE )
    string( JOIN " " tmp ${cmd} )
    message( STATUS "-- Invoking:" ${tmp})
  else()
    list( APPEND cmd ERROR_QUIET )
  endif()
  execute_process( COMMAND ${cmd}
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE cmd_exitcode
    )
  if ( NOT "x${cmd_exitcode}" STREQUAL "x0" )
    message( FATAL_ERROR "Could not find pybind11." )
  endif()
  if ( DG_VERBOSE )
    message( STATUS "-- Found pybind11_DIR=${pybind11_DIR}")
  endif()

  set( cmd "${PYTHON_EXECUTABLE}" -mpybind11 --version )
  if ( DG_VERBOSE )
    string( JOIN " " tmp ${cmd} )
    message( STATUS "-- Invoking:" ${tmp})
  else()
    list( APPEND cmd ERROR_QUIET )
  endif()
  execute_process( COMMAND ${cmd}
    OUTPUT_VARIABLE pybind11_version
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE cmd_exitcode
    )
  if ( NOT "x${cmd_exitcode}" STREQUAL "x0" )
    message( FATAL_ERROR "Could not get version of pybind11." )
  endif()
  set( findpkg_args "pybind11;2.10.4;NO_MODULE;NO_DEFAULT_PATH" )
  set( cmake_args "-DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}" "-Dpybind11_DIR=${pybind11_DIR}" )

  extract_extdep_flags( CXX "${findpkg_args}" "pybind11::module" "${cmake_args}" module_cflags module_linkflags )
  extract_extdep_flags( CXX "${findpkg_args}" "pybind11::embed" "${cmake_args}" embed_cflags embed_linkflags )

  #For some reason the -DUSING_pybind11 define does not get added with the
  #above. Try to add it manually and hope it works:
  set( ${resvar_version} "${pybind11_version}" PARENT_SCOPE )
  set( ${resvar_embed_cflags_list} "${embed_cflags} -DUSING_pybind11" PARENT_SCOPE )
  set( ${resvar_embed_linkflags_list} "${embed_linkflags}" PARENT_SCOPE )
  set( ${resvar_module_cflags_list} "${module_cflags} -DUSING_pybind11" PARENT_SCOPE )
  set( ${resvar_module_linkflags_list} "${module_linkflags}" PARENT_SCOPE )
endfunction()

detect_system_pybind11(
  PYBIND11_EMBED_CFLAGS_LIST
  PYBIND11_EMBED_LINKFLAGS_LIST
  PYBIND11_MODULE_CFLAGS_LIST
  PYBIND11_MODULE_LINKFLAGS_LIST
  PYBIND11_VERSION
  )

message( STATUS "-- Found pybind11 version ${pybind11_version}")

if ( DG_VERBOSE )
  message("-- pybind11 compilation flags (embed): ${PYBIND11_EMBED_CFLAGS_LIST} " )
  message("-- pybind11 link flags (embed): ${PYBIND11_EMBED_LINKFLAGS_LIST} " )
  message("-- pybind11 compilation flags (module): ${PYBIND11_MODULE_CFLAGS_LIST} " )
  message("-- pybind11 link flags (module): ${PYBIND11_MODULE_LINKFLAGS_LIST} " )
endif()

list(APPEND DG_GLOBAL_VERSION_DEPS_CXX "pybind11##${PYBIND11_VERSION}")
list(APPEND DG_GLOBAL_VERSION_DEPS_C "pybind11##${PYBIND11_VERSION}")
