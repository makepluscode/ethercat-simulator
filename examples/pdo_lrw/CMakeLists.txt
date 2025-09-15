add_executable(examples_pdo_lrw
    main.cpp
)

target_link_libraries(examples_pdo_lrw
    PRIVATE
        ethercat_core
        ethercat_kickcat_adapter
        kickcat::kickcat
)

add_test(NAME examples.pdo_lrw_run COMMAND examples_pdo_lrw)
set_tests_properties(examples.pdo_lrw_run PROPERTIES LABELS "examples")
