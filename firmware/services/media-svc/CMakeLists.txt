add_library(media_svc STATIC src/media_svc.cpp)
target_include_directories(media_svc PUBLIC include)
target_link_libraries(media_svc PUBLIC edger_core)
target_compile_options(media_svc PRIVATE -Wall -Wextra)
