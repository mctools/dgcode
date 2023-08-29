# Temporary meta-dependency package which can be used to enable packages only
# when NCrystal is absent on the system.
#
if (NOT "x${HAS_NCrystal}" STREQUAL "xpending" )
  if ( HAS_NCrystal )
    set(HAS_NoSystemNCrystal 0 )
  else()
    set(HAS_NoSystemNCrystal 1 )
    set(ExtDep_NoSystemNCrystal_VERSION "-")
    set(ExtDep_NoSystemNCrystal_COMPILE_FLAGS "")
    set(ExtDep_NoSystemNCrystal_LINK_FLAGS "")
  endif()
endif()
