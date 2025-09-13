cmake_minimum_required(VERSION 3.20)

project(EtherCATSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_GUI "Build GUI applications (Qt6 QML)" OFF)

include(CTest)
enable_testing()

# Optional dependencies via Conan
find_package(fastdds CONFIG QUIET)
if(fastdds_FOUND)
    message(STATUS "FastDDS found via Conan")
    set(HAVE_FASTDDS ON)
endif()

find_package(ftxui CONFIG QUIET)
if(ftxui_FOUND)
    message(STATUS "FTXUI found via Conan")
    set(HAVE_FTXUI ON)
endif()

# KickCAT is required by default (via Conan or vendored fallback)
find_package(kickcat CONFIG)
if(NOT kickcat_FOUND)
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
        set(kickcat_FOUND ON)
    endif()
endif()

if(NOT kickcat_FOUND)
    message(FATAL_ERROR "KickCAT not found. Install via Conan or sync to libs/kickcat.")
endif()

set(HAVE_KICKCAT ON)

add_subdirectory(core)

if(BUILD_EXAMPLES)
    add_subdirectory(examples/hello)
    add_subdirectory(examples/kickcat_scan)
    add_subdirectory(examples/pdo_lrw)
    if(HAVE_FTXUI)
        add_subdirectory(examples/ftxui_demo)
    endif()
    if(HAVE_FASTDDS)
        add_subdirectory(examples/fastdds_smoke)
        add_subdirectory(examples/simul)
    endif()
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_GUI)
    add_subdirectory(gui/desktop)
endif()

# Optional TUI application (requires FTXUI)
if(HAVE_FTXUI)
    add_subdirectory(tui)
endif()
