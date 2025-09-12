########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(kickcat_FIND_QUIETLY)
    set(kickcat_MESSAGE_MODE VERBOSE)
else()
    set(kickcat_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/kickcatTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${kickcat_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(kickcat_VERSION_STRING "v2.0-rc1")
set(kickcat_INCLUDE_DIRS ${kickcat_INCLUDE_DIRS_RELEASE} )
set(kickcat_INCLUDE_DIR ${kickcat_INCLUDE_DIRS_RELEASE} )
set(kickcat_LIBRARIES ${kickcat_LIBRARIES_RELEASE} )
set(kickcat_DEFINITIONS ${kickcat_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${kickcat_BUILD_MODULES_PATHS_RELEASE} )
    message(${kickcat_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


