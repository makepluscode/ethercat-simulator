cmake_minimum_required(VERSION 3.20)

add_executable(a-master
    app/main.cpp
    bus/master_socket.cpp
    mvc/controller/master_controller.cpp
)

target_include_directories(a-master
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(a-master PUBLIC cxx_std_17)
target_link_libraries(a-master
    PRIVATE
        ethercat_core
        kickcat::kickcat
)

if(HAVE_FTXUI)
  target_sources(a-master PRIVATE
    mvc/view/master_tui.cpp
  )
  target_compile_definitions(a-master PRIVATE HAVE_FTXUI=1)
  target_link_libraries(a-master PRIVATE ftxui::screen ftxui::dom ftxui::component)
endif()

install(TARGETS a-master RUNTIME DESTINATION bin)
