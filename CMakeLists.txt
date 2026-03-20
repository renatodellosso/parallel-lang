cmake_minimum_required(VERSION 4.1.1)

project(parallel-lang)

add_executable(ParallelLang)

target_sources(ParallelLang
  PRIVATE
    src/main.cpp
)