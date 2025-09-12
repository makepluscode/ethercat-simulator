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
