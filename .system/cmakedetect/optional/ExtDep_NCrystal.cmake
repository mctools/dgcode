set(HAS_NCrystal 0)

set(autoreconf_bin_NCrystal "ncrystal-config;nctool")
set(autoreconf_env_NCrystal "NCRYSTALDIR;DGCODE_USESYSNCRYSTAL")

function(
    detect_system_ncrystal
    resvar_found
    resvar_version
    resvar_cxx_cflags_list
    resvar_c_cflags_list
    resvar_linkflags_list
    )

  set( ${resvar_found} 0 PARENT_SCOPE )
  set( cmd ncrystal-config --show cmakedir )
  if ( DG_VERBOSE )
    string( JOIN " " tmp ${cmd} )
    message("-- Invoking:" ${tmp})
  else()
    list( APPEND cmd ERROR_QUIET )
  endif()
  execute_process( COMMAND ${cmd}
    OUTPUT_VARIABLE NCrystal_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE cmd_exitcode
    )
  if ( NOT "x${cmd_exitcode}" STREQUAL "x0" )
    return()
  endif()
  set( findpkgargs NCrystal 3.7.1 NO_MODULE NO_DEFAULT_PATH )#TODO: If found but version is too old, and in conda, provide conda command for updating ncrystal version.
  if ( DG_VERBOSE )
    message("-- Found NCrystal_DIR=${NCrystal_DIR}")
    string( JOIN " " tmp ${findpkgargs} )
    message("-- Trying to invoke find_package( ${tmp} )." )
  endif()
  set( preserve_NCrystal_DIR "${NCrystal_DIR}" )#work around bug in NCrystal <= 3.7.1 where the find_package call would override NCrystal_DIR.
  find_package( ${findpkgargs} )
  set( NCrystal_DIR "${preserve_NCrystal_DIR}" )
  if ( NOT NCrystal_FOUND )
    if ( DG_VERBOSE )
      message("-- The find_package call failed.")
    endif()
    return()
  endif()
  if ( DG_VERBOSE )
    message("-- Now trying to detect CXX settings for NCrystal.")
  endif()
  extract_extdep_flags(
    CXX "${findpkgargs}" "NCrystal::NCrystal" "-DNCrystal_DIR=${NCrystal_DIR}"
    ncrystal_cxx_cflags ncrystal_cxx_linkflags
    )
  if ( DG_VERBOSE )
    message("-- Now trying to detect C settings for NCrystal.")
  endif()
  extract_extdep_flags(
    C "${findpkgargs}" "NCrystal::NCrystal" "-DNCrystal_DIR=${NCrystal_DIR}"
    ncrystal_c_cflags ncrystal_c_linkflags
    )
  if ( NOT "x${resvar_c_linkflags_list}" STREQUAL "x${resvar_cxx_linkflags_list}" )
    message("-- ERROR: Found different NCrystal link flags for c++ and c!!")
    return()
  endif()

  #Adding -I${NCrystal_INCDIR} directly to the flags for added robustness, since
  #extract_extdep_flags might miss it if it was already in a default include
  #path for the secondary cmake process:
  set( ncrystal_cxx_cflags "${ncrystal_cxx_cflags} -I${NCrystal_INCDIR}" )
  set( ncrystal_c_cflags "${ncrystal_c_cflags} -I${NCrystal_INCDIR}" )

  #Adding flags, to help with redirection resolution in NCrystalRel headers:
  set( ncrystal_cxx_cflags "${ncrystal_cxx_cflags} -DDGCODE_USE_SYSTEM_NCRYSTAL" )
  set( ncrystal_c_cflags "${ncrystal_c_cflags} -DDGCODE_USE_SYSTEM_NCRYSTAL" )

  set( ${resvar_found} 1 PARENT_SCOPE )
  set( ${resvar_version} "${NCrystal_VERSION}" PARENT_SCOPE )
  set( ${resvar_cxx_cflags_list} "${ncrystal_cxx_cflags}" PARENT_SCOPE )
  set( ${resvar_c_cflags_list} "${ncrystal_c_cflags}" PARENT_SCOPE )
  set( ${resvar_linkflags_list} "${ncrystal_c_linkflags}" PARENT_SCOPE )
endfunction()

set( strategy_sysncrystal "$ENV{DGCODE_USESYSNCRYSTAL}" )
string( TOUPPER "${strategy_sysncrystal}" strategy_sysncrystal )

if ( "x${strategy_sysncrystal}" STREQUAL "x" )
  #Default value ( TODO: Change to "auto" at some point in the future ):
  set( strategy_sysncrystal NEVER )
endif()

set( strategy_sysncrystal_allowed_values "AUTO" "ALWAYS" "NEVER" )
if ( NOT strategy_sysncrystal IN_LIST strategy_sysncrystal_allowed_values )
  message( FATAL_ERROR "Invalid value of DGCODE_USESYSNCRYSTAL env var."
    " Must be unset or one of: ${strategy_sysncrystal_allowed_values}")
endif()

if ( NOT strategy_sysncrystal STREQUAL "NEVER" )
  detect_system_ncrystal( HAS_NCrystal
    ExtDep_NCrystal_VERSION
    ExtDep_NCrystal_COMPILE_FLAGS_CXX
    ExtDep_NCrystal_COMPILE_FLAGS_C
    ExtDep_NCrystal_LINK_FLAGS )
else()
  message( "-- Skipping search for system-provided NCrystal due to value of DGCODE_USESYSNCRYSTAL." )
endif()

if ( NOT HAS_NCrystal AND strategy_sysncrystal STREQUAL "ALWAYS" )
  message( FATAL_ERROR "-- DGCODE_USESYSNCRYSTAL=ALWAYS but could not find a working NCrystal installation." )
endif()
