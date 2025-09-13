add_executable(examples_ftxui
    main.cpp
)

target_link_libraries(examples_ftxui
    PRIVATE
        ftxui::dom
        ftxui::screen
)

add_test(NAME examples.ftxui_run COMMAND examples_ftxui)

