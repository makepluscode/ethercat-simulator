cmake_minimum_required(VERSION 3.20)

project(EtherCATSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable code coverage (gcc/clang)" OFF)
option(ENABLE_LLVM_COVERAGE "Use llvm-cov/llvm-profdata coverage (clang)" OFF)

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_GUI "Build GUI applications (Qt6 QML)" OFF)
option(FORCE_NO_FTXUI "Ignore FTXUI even if found" OFF)
option(FORCE_NO_FASTDDS "Ignore FastDDS even if found" OFF)

include(CTest)
enable_testing()

if(ENABLE_COVERAGE)
    if(ENABLE_LLVM_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message(STATUS "Coverage (LLVM) enabled: -fprofile-instr-generate -fcoverage-mapping")
            add_compile_options(-fprofile-instr-generate -fcoverage-mapping -O0 -g)
            add_link_options(-fprofile-instr-generate)
        else()
            message(WARNING "ENABLE_LLVM_COVERAGE requested, but compiler is not Clang. Falling back to GCC-style flags.")
            add_compile_options(--coverage -O0 -g)
            add_link_options(--coverage)
        endif()
    else()
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            message(STATUS "Coverage enabled: adding --coverage flags")
            add_compile_options(--coverage -O0 -g)
            add_link_options(--coverage)
        else()
            message(WARNING "ENABLE_COVERAGE is ON, but compiler is not GCC/Clang. Skipping flags.")
        endif()
    endif()
endif()

# Optional dependencies via Conan
find_package(fastdds CONFIG QUIET)
if(fastdds_FOUND AND NOT FORCE_NO_FASTDDS)
    message(STATUS "FastDDS found via Conan")
    set(HAVE_FASTDDS ON)
elseif(fastdds_FOUND AND FORCE_NO_FASTDDS)
    message(STATUS "FastDDS found but disabled by FORCE_NO_FASTDDS")
endif()

find_package(ftxui CONFIG QUIET)
if(ftxui_FOUND AND NOT FORCE_NO_FTXUI)
    message(STATUS "FTXUI found via Conan")
    set(HAVE_FTXUI ON)
elseif(ftxui_FOUND AND FORCE_NO_FTXUI)
    message(STATUS "FTXUI found but disabled by FORCE_NO_FTXUI")
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
