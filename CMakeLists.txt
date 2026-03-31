cmake_minimum_required(VERSION 3.16)

project(nanoipc C CXX)

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

add_library(nanoipc INTERFACE)
target_sources(nanoipc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/nanoipc.cpp)
target_include_directories(nanoipc INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanoipc INTERFACE nanocobs_lib)

if(NOT DEFINED NO_TESTS)
    add_subdirectory(tests)
endif()