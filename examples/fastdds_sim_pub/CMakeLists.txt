add_executable(fastdds_sim_pub
    main.cpp
)

target_link_libraries(fastdds_sim_pub
    PRIVATE
        ethercat_dds
        fastdds
)
