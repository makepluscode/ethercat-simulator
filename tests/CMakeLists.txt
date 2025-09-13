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

if(HAVE_KICKCAT)
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
endif()
