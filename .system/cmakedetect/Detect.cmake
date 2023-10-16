######################################################
###   Standard packages which we always require    ###
###   => all code can implicitly depend on these   ###
######################################################

cmake_policy(PUSH)
include("ExtDep_Python.cmake")
cmake_policy(POP)

include( "ExtractFlags.cmake")

# DGBUILD-EXPORT-ONLY>>#####################
# DGBUILD-EXPORT-ONLY>>###   pybind11    ###
# DGBUILD-EXPORT-ONLY>>#####################
# DGBUILD-EXPORT-ONLY>>
# DGBUILD-EXPORT-ONLY>>set( strategy_syspyboost "PYBIND11" )
# DGBUILD-EXPORT-ONLY>>cmake_policy(PUSH)
# DGBUILD-EXPORT-ONLY>>include("ExtDep_pybind11.cmake")
# DGBUILD-EXPORT-ONLY>>cmake_policy(POP)
##########################################################                           # DGBUILD-NO-EXPORT
###   boost-python can either come from our shipped    ###                           # DGBUILD-NO-EXPORT
###   version, or optionally from conda-boost-python   ###                           # DGBUILD-NO-EXPORT
##########################################################                           # DGBUILD-NO-EXPORT
set( strategy_syspyboost "$ENV{DGCODE_USECONDABOOSTPYTHON}" )                        # DGBUILD-NO-EXPORT
string( TOUPPER "${strategy_syspyboost}" strategy_syspyboost )                       # DGBUILD-NO-EXPORT
                                                                                     # DGBUILD-NO-EXPORT
if ( "x${strategy_syspyboost}" STREQUAL "x" )                                        # DGBUILD-NO-EXPORT
  #Default value:                                                                    # DGBUILD-NO-EXPORT
  set( strategy_syspyboost PYBIND11 )                                                # DGBUILD-NO-EXPORT
endif()                                                                              # DGBUILD-NO-EXPORT
                                                                                     # DGBUILD-NO-EXPORT
set( strategy_syspyboost_allowed_values "AUTO" "ALWAYS" "NEVER" "PYBIND11" )         # DGBUILD-NO-EXPORT
if ( NOT strategy_syspyboost IN_LIST strategy_syspyboost_allowed_values )            # DGBUILD-NO-EXPORT
  message( FATAL_ERROR "Invalid value of DGCODE_USECONDABOOSTPYTHON env var."        # DGBUILD-NO-EXPORT
    " Must be unset or one of: ${strategy_syspyboost_allowed_values}")               # DGBUILD-NO-EXPORT
endif()                                                                              # DGBUILD-NO-EXPORT
                                                                                     # DGBUILD-NO-EXPORT
if ( strategy_syspyboost STREQUAL "PYBIND11" )                                       # DGBUILD-NO-EXPORT
  cmake_policy(PUSH)                                                                 # DGBUILD-NO-EXPORT
  include("ExtDep_pybind11.cmake")                                                   # DGBUILD-NO-EXPORT
  cmake_policy(POP)                                                                  # DGBUILD-NO-EXPORT
else()                                                                               # DGBUILD-NO-EXPORT
  cmake_policy(PUSH)                                                                 # DGBUILD-NO-EXPORT
  include("ExtDep_Boost.cmake")                                                      # DGBUILD-NO-EXPORT
  cmake_policy(POP)                                                                  # DGBUILD-NO-EXPORT
endif()                                                                              # DGBUILD-NO-EXPORT


#Declare include dirs added above as -isystem (and remove /usr/include):
declare_includes_as_system_headers(DG_GLOBAL_COMPILE_FLAGS_CXX)
declare_includes_as_system_headers(DG_GLOBAL_COMPILE_FLAGS_C)

#####################################################################
###   Optional dependencies which if absent causes parts of our   ###
###   code to not be built and which the developer of each        ###
###   package must explicitly add as a dependency if needed       ###
#####################################################################

#Create extdep_all list based on files in optional:
FILE(GLOB extdepfiles "optional/ExtDep_*.cmake")
set(extdep_all "")
foreach(extdepfile ${extdepfiles})
  GET_FILENAME_COMPONENT(extdepfile ${extdepfile} NAME)
  string(LENGTH ${extdepfile} tmp)
  MATH(EXPR tmp "${tmp}-13")
  string(SUBSTRING ${extdepfile} 7 ${tmp} extdepfile)
  list(APPEND extdep_all ${extdepfile})
endforeach()
list(SORT extdep_all)

#Go through and treat each in turn:
set(extdep_present "")
set(extdep_missing "")
set(extdep_autoreconf_bin_list "cmake;gcc;g++;clang;clang++;python3")
set(extdep_autoreconf_env_list "CC;CXX;DGDEPDIR;PYTHONHOME;VIRTUAL_ENV;SDKROOT;CPATH")
set(extdep_pending "${extdep_all}")

foreach(extdep ${extdep_all})
    set(HAS_${extdep} "pending")
endforeach()

string(REPLACE ":" ";" DG_ACTUALLY_USED_EXTDEPS "${DG_ACTUALLY_USED_EXTDEPS}")

#Make sure the DG_ACTUALLY_USED_EXTDEPS are first in the list, since they might
#activate other not directly used extdeps via the EXTDEPS_WAITS_FOR variable
#(e.g. Garfield might activate ROOT):
foreach( tmp ${DG_ACTUALLY_USED_EXTDEPS} )
  if ( NOT "${tmp}" IN_LIST extdep_pending )
    message( FATAL_ERROR "Unknown external dependency specified in pkg.info file: \"${tmp}\"" )
  endif()
  list(REMOVE_ITEM extdep_pending ${tmp})
endforeach()
list( PREPEND extdep_pending ${DG_ACTUALLY_USED_EXTDEPS} )

while(extdep_pending)
  list(GET extdep_pending 0 extdep)

  message("-- Checking for ${extdep} installation")
  #We can explicitly ignore a dependency named SomeExtdep by putting "SomeExtdep=0" or "SomeExtdep=OFF", etc.
  if ( NOT "x${${extdep}}" STREQUAL "x"  AND NOT "${${extdep}}" )
    set(HAS_${extdep} 0)
    message( STATUS "Skipping since this dependency was explicitly disabled")
  elseif( NOT "${extdep}" IN_LIST DG_ACTUALLY_USED_EXTDEPS )
    set(HAS_${extdep} 0)
    message( STATUS "Skipping since no active packages require this dependency")
  else()
    cmake_policy(PUSH)
    set( EXTDEPS_WAITS_FOR "")
    include(optional/ExtDep_${extdep}.cmake)
    if ( EXTDEPS_WAITS_FOR )
      list( APPEND DG_ACTUALLY_USED_EXTDEPS ${EXTDEPS_WAITS_FOR} )
    endif()
    cmake_policy(POP)
  endif()

  if ("x${HAS_${extdep}}" STREQUAL "xpending")
    message("-- Checking for ${extdep} installation -- postponed")
    list(REMOVE_ITEM extdep_pending ${extdep})
    list(APPEND extdep_pending ${extdep})
  else()
    if (NOT "x${HAS_${extdep}}" STREQUAL "x0" AND NOT "x${HAS_${extdep}}" STREQUAL "x1")
      message(FATAL_ERROR "Inconsistency in definition of external dependency ${extdep}")
    endif()

    list(APPEND extdep_autoreconf_bin_list ${autoreconf_bin_${extdep}})

    list(APPEND extdep_autoreconf_env_list ${autoreconf_env_${extdep}})

    if (HAS_${extdep})
      list(APPEND extdep_present ${extdep})
      declare_includes_as_system_headers(ExtDep_${extdep}_COMPILE_FLAGS)
      declare_includes_as_system_headers(ExtDep_${extdep}_COMPILE_FLAGS_C)
      declare_includes_as_system_headers(ExtDep_${extdep}_COMPILE_FLAGS_CXX)
      if (ExtDep_${extdep}_COMPILE_FLAGS)
        if (ExtDep_${extdep}_COMPILE_FLAGS_C OR ExtDep_${extdep}_COMPILE_FLAGS_CXX)
          message(FATAL_ERROR "Inconsistency in definition of external dependency ${extdep} (should set either _COMPILE_C+_COMPILE_CXX or just _COMPILE)")
        endif()
        #Copy all flags - but keep those specific to C++ from being applied for C:
        set(ExtDep_${extdep}_COMPILE_FLAGS_CXX "${ExtDep_${extdep}_COMPILE_FLAGS}")
        string(REPLACE " " ";" tmplist "${ExtDep_${extdep}_COMPILE_FLAGS}")
        #set(tmplist "${ExtDep_${extdep}_COMPILE_FLAGS}")#convert from "a b c" to actual list of a b c
        foreach(tmpflag ${tmplist})
          if (NOT tmpflag MATCHES "-W.*-virtual")#FIXME: Also --std=c++ ?
            set(ExtDep_${extdep}_COMPILE_FLAGS_C "${ExtDep_${extdep}_COMPILE_FLAGS_C} ${tmpflag}")
          endif()
        endforeach()
      endif()
      set(ExtDep_${extdep}_COMPILE_FLAGS "")
      message("-- Checking for ${extdep} installation -- yes")
      message("-- Found ${extdep} version: ${ExtDep_${extdep}_VERSION}")
      if (DG_VERBOSE)
        message("-- ${extdep} link flags: ${ExtDep_${extdep}_LINK_FLAGS}")
        message("-- ${extdep} compile flags (C++ only): ${ExtDep_${extdep}_COMPILE_FLAGS_CXX}")
        message("-- ${extdep} compile flags (C/others): ${ExtDep_${extdep}_COMPILE_FLAGS_C}")
      endif()
    else()
      list(APPEND extdep_missing ${extdep})
      message("-- Checking for ${extdep} installation -- no")
    endif()

    list(REMOVE_ITEM extdep_pending ${extdep})
  endif()
endwhile()
