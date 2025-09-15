add_executable(simul
    main.cpp
)

target_link_libraries(simul
    PRIVATE
        ethercat_dds
        fastdds
)
