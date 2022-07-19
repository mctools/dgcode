
set(output_file ${CMAKE_CURRENT_BINARY_DIR}/${output_filename})
file(WRITE ${output_file} "#Dependency information extracted via cmake\n")
set(oa APPEND ${output_file})

file(${oa} "VAR@${}CMAKE_BUILD_TYPE@${CMAKE_BUILD_TYPE}\n")
file(${oa} "VAR@${}PYTHON_EXECUTABLE@${PYTHON_EXECUTABLE}\n")
file(${oa} "VAR@${}PYTHON_LIBRARIES@${PYTHON_LIBRARIES}\n")
file(${oa} "VAR@${}PYTHON_INCLUDE_DIRS@${PYTHON_INCLUDE_DIRS}\n")

file(${oa} "VAR@${}CMAKE_VERSION@${CMAKE_VERSION}\n")
file(${oa} "VAR@${}CMAKE_SYSTEM@${CMAKE_SYSTEM}\n")

file(${oa} "VAR@${}CMAKE_Fortran_OUTPUT_EXTENSION@${CMAKE_Fortran_OUTPUT_EXTENSION}\n")
file(${oa} "VAR@${}CMAKE_CXX_OUTPUT_EXTENSION@${CMAKE_CXX_OUTPUT_EXTENSION}\n")
file(${oa} "VAR@${}CMAKE_C_OUTPUT_EXTENSION@${CMAKE_C_OUTPUT_EXTENSION}\n")

file(${oa} "VAR@${}CMAKE_Fortran_COMPILER@${CMAKE_Fortran_COMPILER}\n")
file(${oa} "VAR@${}CMAKE_Fortran_FLAGS@${CMAKE_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE}}\n")
file(${oa} "VAR@${}CMAKE_CXX_COMPILER@${CMAKE_CXX_COMPILER}\n")

#compiler versions (so we can trigger rebuilds on system upgrades):
set(tmplangs "CXX;C")
if (HAS_Fortran)
  list(APPEND tmplangs "Fortran")
endif()
foreach(lang ${tmplangs})
  if(CMAKE_${lang}_COMPILER_VERSION)
    string(REGEX MATCH "([0-9]*)\\.([0-9]*)\\.([0-9]*)" dummy ${CMAKE_${lang}_COMPILER_VERSION})
    set(TMPS "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
  else()
    #fallback, hoping compiler supports a meaningful -dumpversion (nb: clang supports this but gives wrong info for compatibility reasons)
    execute_process(COMMAND ${CMAKE_${lang}_COMPILER} "-dumpversion" OUTPUT_VARIABLE TMPS  OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE "\n" ";" TMPS "${TMPS}")
  endif()
  execute_process(COMMAND ${CMAKE_${lang}_COMPILER} "--version" OUTPUT_VARIABLE TMPL  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" TMPL "${TMPL}")
  file(${oa} "VAR@${}CMAKE_${lang}_COMPILER_VERSION_SHORT@${CMAKE_${lang}_COMPILER_ID}/${TMPS}\n")
  file(${oa} "VAR@${}CMAKE_${lang}_COMPILER_VERSION_LONG@${TMPL}\n")
  file(${oa} "VAR@${}DG_GLOBAL_VERSION_DEPS_${lang}@${DG_GLOBAL_VERSION_DEPS_${lang}}\n")
endforeach()

file(${oa} "VAR@${}CMAKE_CXX_FLAGS@${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}\n")
file(${oa} "VAR@${}CMAKE_C_COMPILER@${CMAKE_C_COMPILER}\n")
file(${oa} "VAR@${}CMAKE_C_FLAGS@${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}\n")

file(${oa} "VAR@${}CMAKE_SHARED_LIBRARY_PREFIX@${CMAKE_SHARED_LIBRARY_PREFIX}\n")
file(${oa} "VAR@${}CMAKE_SHARED_LIBRARY_SUFFIX@${CMAKE_SHARED_LIBRARY_SUFFIX}\n")

file(${oa} "VAR@${}CMAKE_CXX_LINK_FLAGS@${CMAKE_CXX_LINK_FLAGS}\n")
file(${oa} "VAR@${}CMAKE_C_LINK_FLAGS@${CMAKE_C_LINK_FLAGS} -lm\n")#libstdc++ drags in libm, but in C it might be missing
file(${oa} "VAR@${}CMAKE_Fortran_LINK_FLAGS@${CMAKE_Fortran_LINK_FLAGS}\n")

file(${oa} "VAR@${}DG_GLOBAL_COMPILE_FLAGS_C@${DG_GLOBAL_COMPILE_FLAGS_C}\n")
file(${oa} "VAR@${}DG_GLOBAL_COMPILE_FLAGS_CXX@${DG_GLOBAL_COMPILE_FLAGS_CXX}\n")
file(${oa} "VAR@${}DG_GLOBAL_LINK_FLAGS@${DG_GLOBAL_LINK_FLAGS}\n")
file(${oa} "VAR@${}DG_GLOBAL_LINK_FLAGS_PREPENDED@${DG_GLOBAL_LINK_FLAGS_PREPENDED}\n")

file(${oa} "VAR@${}DG_EXTRA_LDLIBPATHS@${DG_EXTRA_LDLIBPATHS}\n")
file(${oa} "VAR@${}DG_EXTRA_PATHS@${DG_EXTRA_PATHS}\n")
file(${oa} "VAR@${}DG_LIBS_TO_SYMLINK@${DG_LIBS_TO_SYMLINK}\n")

foreach(lang ${tmplangs})
  file(${oa} "VAR@${}RULE_${lang}_SHLIB@${CMAKE_${lang}_COMPILER} ${CMAKE_SHARED_LIBRARY_${lang}_FLAGS} [FLAGS] ${CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS} [INPUT] -o [OUTPUT]\n")
  file(${oa} "VAR@${}RULE_${lang}_COMPOBJ@${CMAKE_${lang}_COMPILER} [FLAGS] -c [INPUT] -o [OUTPUT]\n")
  file(${oa} "VAR@${}RULE_${lang}_EXECUTABLE@${CMAKE_${lang}_COMPILER} [FLAGS] ${CMAKE_${lang}_EXECLINK_FLAGS} ${CMAKE_${lang}_LINK_FLAGS} [INPUT] -o [OUTPUT]\n")
  file(${oa} "VAR@${}RPATH_FLAG_${lang}_EXECUTABLE@${CMAKE_EXECUTABLE_RUNTIME_${lang}_FLAG}\n")
  file(${oa} "VAR@${}RPATH_FLAG_${lang}_SHLIB@${CMAKE_SHARED_LIBRARY_RUNTIME_${lang}_FLAG}\n")
endforeach()

foreach(extdep ${extdep_all})
  if (HAS_${extdep})
    list(APPEND extdep_present ${extdep})
    file(${oa} "EXT@${extdep}@PRESENT@1\n")
    file(${oa} "EXT@${extdep}@LINK@${ExtDep_${extdep}_LINK_FLAGS}\n")
    file(${oa} "EXT@${extdep}@COMPILE_CXX@${ExtDep_${extdep}_COMPILE_FLAGS_CXX}\n")
    file(${oa} "EXT@${extdep}@COMPILE_C@${ExtDep_${extdep}_COMPILE_FLAGS_C}\n")
    file(${oa} "EXT@${extdep}@VERSION@${ExtDep_${extdep}_VERSION}\n")
  else()
    file(${oa} "EXT@${extdep}@PRESENT@0\n")
  endif()
endforeach()

file(${oa} "VAR@${}autoreconf_bin_list@${extdep_autoreconf_bin_list}\n")
file(${oa} "VAR@${}autoreconf_env_list@${extdep_autoreconf_env_list}\n")

