add_executable(tui
    src/main.cpp
)

target_link_libraries(tui
    PRIVATE
        ftxui::dom
        ftxui::screen
)

target_link_libraries(tui PRIVATE ethercat_dds fastdds::fastdds)
