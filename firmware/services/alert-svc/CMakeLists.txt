add_library(alert_svc STATIC src/alert_svc.cpp)
target_include_directories(alert_svc PUBLIC include)
target_link_libraries(alert_svc PUBLIC edger_core)
target_compile_options(alert_svc PRIVATE -Wall -Wextra)
