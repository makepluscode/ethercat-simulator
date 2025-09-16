add_library(ethercat_core STATIC
    simulation/network_simulator.cpp
)

target_include_directories(ethercat_core
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../include
)

target_compile_features(ethercat_core PUBLIC cxx_std_17)
target_link_libraries(ethercat_core
    PUBLIC
        kickcat::kickcat
)

if(HAVE_KICKCAT)
    add_library(ethercat_kickcat_adapter STATIC
        kickcat/sim_socket.cpp
    )
    target_include_directories(ethercat_kickcat_adapter
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/../include
    )
    target_link_libraries(ethercat_kickcat_adapter
        PUBLIC
            ethercat_core
            kickcat::kickcat
    )
    target_compile_features(ethercat_kickcat_adapter PUBLIC cxx_std_17)
endif()

if(HAVE_FASTDDS)
    add_library(ethercat_dds STATIC
        communication/dds_text_type.cpp
    )
    target_include_directories(ethercat_dds
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/../include
    )
    target_link_libraries(ethercat_dds
        PUBLIC
            fastdds
    )
    target_compile_features(ethercat_dds PUBLIC cxx_std_17)
endif()
