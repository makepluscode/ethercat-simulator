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
