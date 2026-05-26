add_library(edger_core
  src/config.cpp
)

target_include_directories(edger_core
  PUBLIC include
)

target_compile_options(edger_core PRIVATE -Wall -Wextra)
