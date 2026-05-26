add_library(record_svc STATIC src/record_svc.cpp)
target_include_directories(record_svc PUBLIC include)
target_link_libraries(record_svc PUBLIC edger_core)
target_compile_options(record_svc PRIVATE -Wall -Wextra)
