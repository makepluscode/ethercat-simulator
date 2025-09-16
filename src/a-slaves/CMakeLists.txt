cmake_minimum_required(VERSION 3.20)

add_executable(a-slaves
    app/main.cpp
    bus/slave_endpoint.cpp
    mvc/controller/slave_controller.cpp
)

target_include_directories(a-slaves
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(a-slaves PUBLIC cxx_std_17)
target_link_libraries(a-slaves
    PRIVATE
        ethercat_core
        kickcat::kickcat
)

if(HAVE_FTXUI)
  target_sources(a-slaves PRIVATE
    mvc/view/slave_tui.cpp
  )
  target_compile_definitions(a-slaves PRIVATE HAVE_FTXUI=1)
  target_link_libraries(a-slaves PRIVATE ftxui::screen ftxui::dom ftxui::component)
endif()

install(TARGETS a-slaves RUNTIME DESTINATION bin)
