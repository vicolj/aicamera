cmake_minimum_required(VERSION 3.16)
project(edger-rec VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED EDGER_BOARD)
  set(EDGER_BOARD "x86")
endif()

option(EDGER_BUILD_QT "Build Qt config app" OFF)
option(EDGER_BUILD_WEB "Build web server" OFF)

add_subdirectory(edger-core)
add_subdirectory(services/media-svc)
add_subdirectory(services/record-svc)
add_subdirectory(services/ai-svc)
add_subdirectory(services/alert-svc)

add_executable(edger-rec-demo apps/demo/main.cpp)
target_link_libraries(edger-rec-demo PRIVATE edger_core media_svc record_svc ai_svc alert_svc)
target_compile_definitions(edger-rec-demo PRIVATE "EDGER_BOARD=\"${EDGER_BOARD}\"")

install(TARGETS edger-rec-demo edger_core media_svc record_svc ai_svc alert_svc
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
