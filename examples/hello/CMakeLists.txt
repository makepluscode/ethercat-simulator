add_executable(examples_hello
    main.cpp
)

target_link_libraries(examples_hello PRIVATE ethercat_core)

add_test(NAME examples.hello_run COMMAND examples_hello)

