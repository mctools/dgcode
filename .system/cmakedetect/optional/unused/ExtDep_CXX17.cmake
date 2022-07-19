#pseudo-external which is intended to be present only for C++ compilers which
#can be expected to implement complete support for C++17.

set(HAS_CXX17 0)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0)
    set(HAS_CXX17 1)
  endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.0)
    set(HAS_CXX17 1)
  endif()
endif()

if (HAS_CXX17)
  set(ExtDep_CXX17_COMPILE_FLAGS "")#no need to set -std=c++17 since we anyway have it on by default
  set(ExtDep_CXX17_LINK_FLAGS "")
  set(ExtDep_CXX17_VERSION "-")#a version here doesn't really make sense
endif()

