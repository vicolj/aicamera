add_library(ai_svc STATIC src/ai_svc.cpp)
target_include_directories(ai_svc PUBLIC include)
target_link_libraries(ai_svc PUBLIC edger_core)
target_compile_options(ai_svc PRIVATE -Wall -Wextra)
