cmake_minimum_required(VERSION 3.20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick Qml REQUIRED)

add_executable(gui_desktop
    src/main.cpp
    qml/qml.qrc
)

target_link_libraries(gui_desktop
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        ethercat_core
)

add_test(NAME gui.desktop_smoke COMMAND gui_desktop --smoke)

