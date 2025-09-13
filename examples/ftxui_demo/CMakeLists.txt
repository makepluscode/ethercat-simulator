add_executable(examples_ftxui
    main.cpp
)

target_link_libraries(examples_ftxui
    PRIVATE
        ftxui::dom
        ftxui::screen
)


