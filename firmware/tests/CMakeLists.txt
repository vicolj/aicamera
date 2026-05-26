cmake_minimum_required(VERSION 3.16)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(edger_tests
  test_week1_config.cpp
  test_week2_index.cpp
  test_week2_retention.cpp
  test_week2_record_options.cpp
)

target_link_libraries(edger_tests
  PRIVATE
    edger_core
    record_svc
    GTest::gtest_main
)

target_compile_options(edger_tests PRIVATE -Wall -Wextra)

include(GoogleTest)
gtest_discover_tests(edger_tests
  PROPERTIES WORKING_DIRECTORY ${EDGER_REPO_ROOT}
)
