include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(nlohmann_json)

add_library(edger_core
  src/config.cpp
  src/config_reload.cpp
  src/fs.cpp
  src/record_index.cpp
  src/polygon.cpp
  src/web_auth.cpp
)

target_include_directories(edger_core
  PUBLIC include
)

target_link_libraries(edger_core
  PUBLIC nlohmann_json::nlohmann_json
)

target_compile_options(edger_core PRIVATE -Wall -Wextra)
