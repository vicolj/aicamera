find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavformat
  libavcodec
  libavutil
)

add_library(media_svc STATIC src/media_svc.cpp)
target_include_directories(media_svc PUBLIC include)
target_link_libraries(media_svc PUBLIC edger_core PkgConfig::FFMPEG)
target_compile_options(media_svc PRIVATE -Wall -Wextra)
