cmake_minimum_required(VERSION 3.20)

add_executable(master
    app/main.cpp
    bus/master_socket.cpp
    logic/master_controller.cpp
)

target_include_directories(master
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(master PUBLIC cxx_std_17)
target_link_libraries(master
    PRIVATE
        ethercat_core
        kickcat::kickcat
)

if(HAVE_FTXUI)
  target_sources(master PRIVATE
    gui/master_tui.cpp
  )
  target_compile_definitions(master PRIVATE HAVE_FTXUI=1)
  target_link_libraries(master PRIVATE ftxui::screen ftxui::dom ftxui::component)
endif()

install(TARGETS master RUNTIME DESTINATION bin)
