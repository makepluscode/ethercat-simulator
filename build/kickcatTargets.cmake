# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/kickcat-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${kickcat_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${kickcat_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET kickcat::kickcat)
    add_library(kickcat::kickcat INTERFACE IMPORTED)
    message(${kickcat_MESSAGE_MODE} "Conan: Target declared 'kickcat::kickcat'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/kickcat-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()