# NB: We do not use the FindPackage machinery for the PythonLibs since it can
# lead to inconsistencies with the detected PythonInterp. Instead we find just
# the PythonInterp and from there we figure out the associated include and lib
# paths.

#find python from PATH, preferring always python2 (in any case, this will be
#symlinked to "python"

# (TODO: dgbuild should accept PYTHON=... option!)

#Reconfigure when python version changed
set(autoreconf_bin_Python "python3")

set(autoreconf_env_Python "PYTHONPATH;PYTHONHOME")


EXECUTE_PROCESS(COMMAND "which" "python3"
                OUTPUT_VARIABLE PYTHON_EXECUTABLE
                RESULT_VARIABLE tmp_ec
                ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "x${tmp_ec}" STREQUAL "x0" OR NOT EXISTS "${PYTHON_EXECUTABLE}")
  message(FATAL_ERROR "No python3 interpreter in PATH")
endif()

get_filename_component(PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE}" REALPATH)
EXECUTE_PROCESS(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import sys;print(sys.version_info[0])"
                OUTPUT_VARIABLE PYTHON_VERSION_MAJOR RESULT_VARIABLE tmp_ec ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "x${tmp_ec}" STREQUAL "x0")
  message(FATAL_ERROR "Unable to determine python major version")
endif()
EXECUTE_PROCESS(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import sys;print(sys.version_info[1])"
                OUTPUT_VARIABLE PYTHON_VERSION_MINOR RESULT_VARIABLE tmp_ec ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "x${tmp_ec}" STREQUAL "x0")
  message(FATAL_ERROR "Unable to determine python minor version")
endif()
EXECUTE_PROCESS(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import sys;print(sys.version_info[2])"
                OUTPUT_VARIABLE PYTHON_VERSION_PATCH RESULT_VARIABLE tmp_ec ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "x${tmp_ec}" STREQUAL "x0")
  message(FATAL_ERROR "Unable to determine python patch version")
endif()
set(PYTHON_VERSION_STRING "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}.${PYTHON_VERSION_PATCH}")
set(tmp_errmsg FATAL_ERROR "Python version incompatible (found ${PYTHON_VERSION_STRING} required is python3.5 or later)")
if (NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
  message(${tmp_errmsg})
endif()
if (${PYTHON_VERSION_MINOR} LESS 5)
  message(${tmp_errmsg})
endif()
message("-- Python version: ${PYTHON_VERSION_STRING}")
set(PYTHONINTERP_FOUND 1)
list(APPEND DG_GLOBAL_VERSION_DEPS_CXX "Python##${PYTHON_VERSION_STRING}")
list(APPEND DG_GLOBAL_VERSION_DEPS_C "Python##${PYTHON_VERSION_STRING}")

get_filename_component(PYTHON_EXECUTABLE_DIR "${PYTHON_EXECUTABLE}" PATH)
get_filename_component(PYTHON_EXECUTABLE_REALNAME "${PYTHON_EXECUTABLE}" NAME)
#Obsolete, no longer valid (gives problems at centos8 from dgdepfixer venv):
#if (NOT EXISTS "${PYTHON_EXECUTABLE_DIR}/python3")
#  message(FATAL_ERROR "Problems with python installation: No python3 alias exists.")
#endif()
message("-- Real python executable: ${PYTHON_EXECUTABLE}")
#set(PYTHON_EXECUTABLE "${PYTHON_EXECUTABLE_DIR}/python")

#find python include path
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} "-c" "from distutils.sysconfig import get_python_inc; print(get_python_inc())"
                OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS
                RESULT_VARIABLE tmp_ec
                ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
###if (NOT "x${tmp_ec}" STREQUAL "x0")
###    #distutils might not be there, but at least in py2.7 on ubuntu
###    #there is a global sysconfig module instead:
###    EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} "-c" "import sysconfig;sysconfig.get_config_var('INCLUDEPY')"
###                    OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS
###                    RESULT_VARIABLE tmp_ec
###                    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
###endif()
if (NOT "x${tmp_ec}" STREQUAL "x0")
  message(FATAL_ERROR "Problems finding python include directories from python interpreter ${PYTHON_EXECUTABLE}")
endif()
message("-- Python include dir: ${PYTHON_INCLUDE_DIRS}")
if (NOT EXISTS "${PYTHON_INCLUDE_DIRS}/Python.h")
  message(FATAL_ERROR "Did not find python header file Python.h. Did you install python3-dev / python3-devel packages?")
endif()
#find python library:

if(EXISTS /proc/cpuinfo)
  set(ldcmd ldd)
  set(ldopt -r)
  set(awkn 3)
else()
  set(ldcmd otool)
  set(ldopt -L)
  set(awkn 1)
endif()

#-iframework/usr/local/Cellar/python/2.7.3/Frameworks/Python.framework -framework Python
#python -c 'from distutils import sysconfig;print(sysconfig.get_config_var("PYTHONFRAMEWORKPREFIX"))'
#/usr/local/Cellar/python/2.7.3/Frameworks
#python -c 'from distutils import sysconfig;print(sysconfig.get_config_var("PYTHONFRAMEWORKDIR"))'
#Python.framework
#python -c 'from distutils import sysconfig;print(sysconfig.get_config_var("PYTHONFRAMEWORK"))'
#Python


#if ("${PYTHON_EXECUTABLE}" MATCHES "/Frameworks/")
#  message("-- It seems that python is delivered by an osx framework")
#  set(PYTHON_FROM_FRAMEWORK 1)
#else()
#  set(PYTHON_FROM_FRAMEWORK 0)
#endif()

set(PYTHON_FROM_FRAMEWORK 0)

#first try python sysconfig:
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} "-c" "import sysconfig as s; print(s.get_config_var('LIBDIR')+'/'+s.get_config_var('LDLIBRARY'))"
                OUTPUT_VARIABLE PYTHON_LIBRARIES
                RESULT_VARIABLE tmp_ec
                ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "x${tmp_ec}" STREQUAL "x0" OR NOT EXISTS "${PYTHON_LIBRARIES}")
  #Try with LIBPL instead of LIBDIR:
  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} "-c" "import sysconfig as s; print(s.get_config_var('LIBPL')+'/'+s.get_config_var('LDLIBRARY'))"
                  OUTPUT_VARIABLE PYTHON_LIBRARIES
                  RESULT_VARIABLE tmp_ec
                  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if (NOT "x${tmp_ec}" STREQUAL "x0" OR NOT EXISTS "${PYTHON_LIBRARIES}")
  #try a bit of mucking together
  EXECUTE_PROCESS(COMMAND ${ldcmd} ${ldopt} "${PYTHON_EXECUTABLE}"
                  COMMAND grep libpython
                  COMMAND head -1
                  COMMAND awk "{print $${awkn}}"
                  OUTPUT_VARIABLE PYTHON_LIBRARIES
                  RESULT_VARIABLE tmp_ec
                  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (NOT "x${tmp_ec}" STREQUAL "x0" OR NOT EXISTS "${PYTHON_LIBRARIES}")
    if ("x${PYTHON_EXECUTABLE_DIR}" STREQUAL "x/usr/bin")
      set(PYTHON_LIBRARIES "/usr/lib/lib${PYTHON_EXECUTABLE_REALNAME}.so")
      if (EXISTS "${PYTHON_LIBRARIES}")
        set(tmp_ec 0)#workaround to at least make ubuntu work...
      endif()
    endif()
  endif()
  if (NOT "x${tmp_ec}" STREQUAL "x0")
    message(FATAL_ERROR "Problems finding python libraries from python interpreter ${PYTHON_EXECUTABLE}")
  endif()
endif()

if (NOT EXISTS "${PYTHON_LIBRARIES}")
  #try to grep for a framework rather than python lib dependency
  EXECUTE_PROCESS(COMMAND ${ldcmd} ${ldopt} "${PYTHON_EXECUTABLE}"
                  COMMAND grep -v /bin/
                  COMMAND grep /Python.framework/
                  COMMAND head -1
                  COMMAND awk "{print $${awkn}}"
                  OUTPUT_VARIABLE PYTHON_LIBRARIES
                  RESULT_VARIABLE tmp_ec
                  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (NOT "x${tmp_ec}" STREQUAL "x0")
    message(FATAL_ERROR "Problems finding python framework library from python interpreter ${PYTHON_EXECUTABLE}")
  endif()
  set(PYTHON_FROM_FRAMEWORK 1)
endif()
if (NOT EXISTS "${PYTHON_LIBRARIES}")
  message(FATAL_ERROR "Problems finding python libraries from python interpreter ${PYTHON_EXECUTABLE}")
endif()
message("-- Python library: ${PYTHON_LIBRARIES}")

set(PYTHONLIBS_FOUND 1)
set(PYTHONLIBS_VERSION_STRING "${PYTHON_VERSION_STRING}")

if (PYTHON_FROM_FRAMEWORK)
  set(DG_GLOBAL_LINK_FLAGS "${DG_GLOBAL_LINK_FLAGS} ${PYTHON_LIBRARIES}")
else()
  findpackage_liblist_to_flags("${PYTHON_LIBRARIES}" "${PYTHON_LIBRARY_DIRS}" PYTHON_LINK_FLAGS)
  set(DG_GLOBAL_LINK_FLAGS "${DG_GLOBAL_LINK_FLAGS} ${PYTHON_LINK_FLAGS}")
endif()
set(DG_GLOBAL_COMPILE_FLAGS_CXX "${DG_GLOBAL_COMPILE_FLAGS_CXX} -I${PYTHON_INCLUDE_DIRS}")
set(DG_GLOBAL_COMPILE_FLAGS_C "${DG_GLOBAL_COMPILE_FLAGS_C} -I${PYTHON_INCLUDE_DIRS}")
if (NOT "x${PYTHONLIBS_VERSION_STRING}" STREQUAL "x${PYTHON_VERSION_STRING}")
  message("WARNING: Python executable and libraries versions might be incompatible!")
endif()
#make sure chosen python lib is put in LD_LIBRARY_PATH of installed setup.sh
#(the executable will be symlinked into dgcode's sysbin later):
if (PYTHON_LIBRARIES)
  list(APPEND DG_LIBS_TO_SYMLINK "${PYTHON_LIBRARIES}")
endif()

