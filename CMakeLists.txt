cmake_minimum_required(VERSION 3.16)

project(digital_twin C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  nanocobs
  GIT_REPOSITORY https://github.com/charlesnicholson/nanocobs.git
  GIT_TAG        v0.2.0
)

FetchContent_MakeAvailable(nanocobs)

add_library(nanocobs_lib INTERFACE)
target_sources(nanocobs_lib INTERFACE ${nanocobs_SOURCE_DIR}/cobs.c)
target_include_directories(nanocobs_lib INTERFACE ${nanocobs_SOURCE_DIR})

add_executable(cobs_test ${CMAKE_CURRENT_LIST_DIR}/src/cobs_test.cpp)
target_link_libraries(cobs_test nanocobs_lib)