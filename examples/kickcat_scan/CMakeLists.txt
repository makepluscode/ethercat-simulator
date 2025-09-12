add_executable(examples_kickcat_scan
    main.cpp
)

target_link_libraries(examples_kickcat_scan
    PRIVATE
        ethercat_core
        ethercat_kickcat_adapter
        kickcat::kickcat
)

add_test(NAME examples.kickcat_scan_run COMMAND examples_kickcat_scan)

