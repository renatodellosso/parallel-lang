cmake_minimum_required(VERSION 4.1.1)

project(parallel-lang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ParallelLang)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.cpp)

target_sources(ParallelLang
  PRIVATE
    ${sources}
)