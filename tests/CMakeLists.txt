find_package(GTest REQUIRED)

add_executable(test_network_simulator
    simulation/test_network_simulator.cpp
)

target_link_libraries(test_network_simulator
    PRIVATE
        ethercat_core
        GTest::gtest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_network_simulator)

add_executable(test_slave_registers
    simulation/test_slave_registers.cpp
)
target_link_libraries(test_slave_registers
    PRIVATE
        ethercat_core
        GTest::gtest
        GTest::gtest_main
)
gtest_discover_tests(test_slave_registers)

add_executable(test_al_status_codes
    simulation/test_al_status_codes.cpp
)
target_link_libraries(test_al_status_codes
    PRIVATE
        ethercat_core
        GTest::gtest
        GTest::gtest_main
)
gtest_discover_tests(test_al_status_codes)

add_executable(test_simulator_registry
    simulation/test_simulator_registry.cpp
)
target_link_libraries(test_simulator_registry
    PRIVATE
        ethercat_core
        GTest::gtest
        GTest::gtest_main
)
gtest_discover_tests(test_simulator_registry)

add_executable(test_mailbox
    simulation/test_mailbox.cpp
)
target_link_libraries(test_mailbox
    PRIVATE
        ethercat_core
        GTest::gtest
        GTest::gtest_main
)
gtest_discover_tests(test_mailbox)

if(HAVE_KICKCAT)
    add_executable(test_kickcat_bus_minimal
        kickcat/test_bus_minimal.cpp
    )
    target_link_libraries(test_kickcat_bus_minimal
        PRIVATE
            ethercat_core
            ethercat_kickcat_adapter
            kickcat::kickcat
            GTest::gtest
            GTest::gtest_main
    )
    gtest_discover_tests(test_kickcat_bus_minimal)

    add_executable(test_kickcat_bus_init
        kickcat/test_bus_init.cpp
    )
    target_link_libraries(test_kickcat_bus_init
        PRIVATE
            ethercat_core
            ethercat_kickcat_adapter
            kickcat::kickcat
            GTest::gtest
            GTest::gtest_main
    )
    gtest_discover_tests(test_kickcat_bus_init)
    add_executable(test_kickcat_sdo
        kickcat/test_sdo.cpp
    )
    target_link_libraries(test_kickcat_sdo
        PRIVATE
            ethercat_core
            ethercat_kickcat_adapter
            kickcat::kickcat
            GTest::gtest
            GTest::gtest_main
    )
    gtest_discover_tests(test_kickcat_sdo)

    add_executable(test_kickcat_adapter
        kickcat/test_kickcat_adapter.cpp
    )
    target_link_libraries(test_kickcat_adapter
        PRIVATE
            ethercat_core
            ethercat_kickcat_adapter
            kickcat::kickcat
            GTest::gtest
            GTest::gtest_main
    )
    gtest_discover_tests(test_kickcat_adapter)

    add_executable(test_kickcat_apr_lr
        kickcat/test_apr_lr.cpp
    )
    target_link_libraries(test_kickcat_apr_lr
        PRIVATE
            ethercat_core
            ethercat_kickcat_adapter
            kickcat::kickcat
            GTest::gtest
            GTest::gtest_main
    )
    gtest_discover_tests(test_kickcat_apr_lr)
endif()

# EL1258 slave tests
add_executable(test_el1258
    simulation/test_el1258.cpp
)
target_link_libraries(test_el1258
    PRIVATE
        ethercat_core
        GTest::gtest
        GTest::gtest_main
)
gtest_discover_tests(test_el1258)

# DDS pub/sub test (FastDDS + Shared Memory only)
if(HAVE_FASTDDS)
    add_executable(test_dds_text_pubsub
        dds/test_text_msg_pubsub.cpp
    )
    target_link_libraries(test_dds_text_pubsub
        PRIVATE
            ethercat_dds
            fastdds
            GTest::gtest
            GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(test_dds_text_pubsub)

    # Bounds/truncation behavior when message exceeds TEXTMSG_MAX_LEN
    add_executable(test_dds_text_bounds
        dds/test_text_msg_bounds.cpp
    )
    target_link_libraries(test_dds_text_bounds
        PRIVATE
            ethercat_dds
            fastdds
            GTest::gtest
            GTest::gtest_main
    )
    gtest_discover_tests(test_dds_text_bounds)
endif()
