cmake_minimum_required(VERSION 3.16)

project(nanoipc C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nanoipc_utils)

add_library(nanoipc_read_buffer INTERFACE)
target_include_directories(nanoipc_read_buffer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(nanoipc_reader INTERFACE)
target_include_directories(nanoipc_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanoipc_reader INTERFACE nanoipc_utils nanoipc_read_buffer)

add_library(nanoipc_writer INTERFACE)
target_include_directories(nanoipc_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanoipc_writer INTERFACE nanoipc_utils)

option(BUILD_TESTS "Build the project's test suite" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()