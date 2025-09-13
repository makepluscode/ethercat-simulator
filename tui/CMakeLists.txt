add_executable(tui
    src/main.cpp
)

target_link_libraries(tui
    PRIVATE
        ftxui::dom
        ftxui::screen
)

if(HAVE_FASTDDS)
    target_link_libraries(tui PRIVATE ethercat_dds fastdds::fastdds)
    target_compile_definitions(tui PRIVATE ETHERCAT_HAVE_FASTDDS)
endif()
