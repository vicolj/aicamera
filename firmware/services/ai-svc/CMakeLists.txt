find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libavutil
  libswscale
)

add_library(ai_svc STATIC
  src/ai_svc.cpp
  src/intrusion_detector.cpp
  src/ai_manager.cpp
)
target_include_directories(ai_svc PUBLIC include)
target_link_libraries(ai_svc PUBLIC edger_core alert_svc PkgConfig::FFMPEG)
target_compile_options(ai_svc PRIVATE -Wall -Wextra)

add_executable(edger-ai-svc apps/ai-svc/main.cpp)
target_link_libraries(edger-ai-svc PRIVATE ai_svc edger_core alert_svc)
set_target_properties(edger-ai-svc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
