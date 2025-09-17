cmake_minimum_required(VERSION 3.20)

add_executable(a-subs
    app/main.cpp
    bus/subs_endpoint.cpp
    mvc/controller/subs_controller.cpp
)

target_include_directories(a-subs
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_features(a-subs PUBLIC cxx_std_17)
target_link_libraries(a-subs
    PRIVATE
        ethercat_core
        kickcat::kickcat
)

if(HAVE_FTXUI)
  target_sources(a-subs PRIVATE
    mvc/view/subs_tui.cpp
  )
  target_compile_definitions(a-subs PRIVATE HAVE_FTXUI=1)
  target_link_libraries(a-subs PRIVATE ftxui::screen ftxui::dom ftxui::component)
endif()

install(TARGETS a-subs RUNTIME DESTINATION bin)
