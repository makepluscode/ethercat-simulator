add_executable(tui
    src/main.cpp
)

target_link_libraries(tui
    PRIVATE
        ftxui::dom
        ftxui::screen
)

target_link_libraries(tui PRIVATE ethercat_dds fastdds::fastdds)

# Register a simple smoke test for the TUI app
if(BUILD_TESTING)
    add_test(NAME tui.Smoke_Run_ExitsZero COMMAND $<TARGET_FILE:tui>)
    set_tests_properties(tui.Smoke_Run_ExitsZero PROPERTIES TIMEOUT 10)
endif()
