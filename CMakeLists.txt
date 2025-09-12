cmake_minimum_required(VERSION 3.20)

project(EtherCATSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLES "Build examples" ON)

include(CTest)
enable_testing()

# Try to locate KickCAT from Conan CMakeDeps (optional)
find_package(kickcat CONFIG QUIET)
if(kickcat_FOUND)
    message(STATUS "KickCAT found via Conan")
    set(HAVE_KICKCAT ON)
else()
    # Fallback to vendored local copy at libs/kickcat
    set(_KICKCAT_LIB "${CMAKE_SOURCE_DIR}/libs/kickcat/lib/libkickcat.a")
    set(_KICKCAT_INC "${CMAKE_SOURCE_DIR}/libs/kickcat/include")
    if(EXISTS "${_KICKCAT_LIB}" AND EXISTS "${_KICKCAT_INC}")
        add_library(kickcat::kickcat STATIC IMPORTED)
        set_target_properties(kickcat::kickcat PROPERTIES
            IMPORTED_LOCATION "${_KICKCAT_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${_KICKCAT_INC}"
        )
        message(STATUS "KickCAT found in libs/kickcat (vendored)")
        set(HAVE_KICKCAT ON)
    endif()
endif()

add_subdirectory(core)

if(BUILD_EXAMPLES)
    add_subdirectory(examples/hello)
    if(HAVE_KICKCAT)
        add_subdirectory(examples/kickcat_scan)
    endif()
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
