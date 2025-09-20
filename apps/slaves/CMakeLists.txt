cmake_minimum_required(VERSION 3.20)

add_executable(slaves
    app/main.cpp
    bus/slaves_endpoint.cpp
    logic/slaves_controller.cpp
)

target_include_directories(slaves
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/core
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(slaves PUBLIC cxx_std_17)
target_link_libraries(slaves
    PRIVATE
        ethercat_core
        ethercat_sim_logger
        kickcat::kickcat
)

if(HAVE_FTXUI)
  target_sources(slaves PRIVATE
    gui/slaves_tui.cpp
  )
  target_compile_definitions(slaves PRIVATE HAVE_FTXUI=1)
  target_link_libraries(slaves PRIVATE ftxui::screen ftxui::dom ftxui::component)
endif()

install(TARGETS slaves RUNTIME DESTINATION bin)
