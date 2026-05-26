cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.18.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(httplib)

add_executable(edger-web-server
  main.cpp
  web_server.cpp
)

target_include_directories(edger-web-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(edger-web-server PRIVATE
  EDGER_WEB_STATIC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/static"
)

target_link_libraries(edger-web-server PRIVATE
  edger_core
  httplib::httplib
)

set_target_properties(edger-web-server PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS edger-web-server RUNTIME DESTINATION bin)
install(DIRECTORY static/ DESTINATION share/edger-rec/web)
