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

add_library(nanoipc_read_buffer INTERFACE)
target_include_directories(nanoipc_read_buffer INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(nanoipc_utils INTERFACE)
target_sources(nanoipc_utils INTERFACE ${CMAKE_CURRENT_LIST_DIR}/nanoipc_utils.cpp ${nanocobs_SOURCE_DIR}/cobs.c)
target_include_directories(nanoipc_utils INTERFACE ${CMAKE_CURRENT_LIST_DIR} ${nanocobs_SOURCE_DIR})
target_link_libraries(nanoipc_utils INTERFACE nanoipc_read_buffer)

add_library(nanoipc_server INTERFACE)
target_include_directories(nanoipc_server INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanoipc_server INTERFACE nanoipc_utils nanoipc_read_buffer)

add_library(nanoipc_server_lib STATIC)
target_sources(nanoipc_server_lib PRIVATE ${CMAKE_CURRENT_LIST_DIR}/nanoipc_server.cpp)
target_link_libraries(nanoipc_server_lib PUBLIC nanoipc_server)

option(BUILD_TESTS "Build the project's test suite" ON)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()