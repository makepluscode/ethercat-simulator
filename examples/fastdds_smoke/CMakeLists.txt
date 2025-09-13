add_executable(examples_fastdds_smoke
    main.cpp
)

target_link_libraries(examples_fastdds_smoke
    PRIVATE
        fastdds::fastdds
)

add_test(NAME examples.fastdds_smoke_run COMMAND examples_fastdds_smoke)

