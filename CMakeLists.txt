cmake_minimum_required(VERSION 4.1.1)

project(parallel-lang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(CLI)
add_executable(Tests)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS tests/*.cpp)

target_sources(CLI
  PRIVATE
    ${sources}
    src/main.cpp
)

target_sources(Tests
  PRIVATE
    ${sources}
    ${test_sources}
)

target_link_libraries(Tests PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(Tests)