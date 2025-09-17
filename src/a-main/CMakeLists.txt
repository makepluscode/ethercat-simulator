cmake_minimum_required(VERSION 3.20)

add_executable(a-main
    app/main.cpp
    bus/main_socket.cpp
    mvc/controller/main_controller.cpp
)

target_include_directories(a-main
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(a-main PUBLIC cxx_std_17)
target_link_libraries(a-main
    PRIVATE
        ethercat_core
        kickcat::kickcat
)

if(HAVE_FTXUI)
  target_sources(a-main PRIVATE
    mvc/view/main_tui.cpp
  )
  target_compile_definitions(a-main PRIVATE HAVE_FTXUI=1)
  target_link_libraries(a-main PRIVATE ftxui::screen ftxui::dom ftxui::component)
endif()

install(TARGETS a-main RUNTIME DESTINATION bin)
