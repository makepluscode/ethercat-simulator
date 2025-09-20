# Find required packages
find_package(Threads REQUIRED)

add_library(ethercat_sim_logger
    logger.cpp
)

target_include_directories(ethercat_sim_logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(ethercat_sim_logger PUBLIC cxx_std_17)

target_link_libraries(ethercat_sim_logger
    PUBLIC
        Threads::Threads
)

# Test executable
add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger ethercat_sim_logger)

# Install the library
install(TARGETS ethercat_sim_logger
    EXPORT ethercat_sim_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)