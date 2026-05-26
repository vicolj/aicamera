find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libavutil
)

add_library(record_svc STATIC
  src/record_svc.cpp
  src/record_manager.cpp
)
target_include_directories(record_svc PUBLIC include)
target_link_libraries(record_svc PUBLIC edger_core PkgConfig::FFMPEG)
target_compile_options(record_svc PRIVATE -Wall -Wextra)

add_executable(edger-record-svc apps/record-svc/main.cpp)
target_link_libraries(edger-record-svc PRIVATE record_svc edger_core)
set_target_properties(edger-record-svc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
