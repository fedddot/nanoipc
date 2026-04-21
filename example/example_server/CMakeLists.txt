cmake_minimum_required(VERSION 3.16)

set(PICO_SDK_FETCH_FROM_GIT ON)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

include(${CMAKE_CURRENT_LIST_DIR}/external/pico_sdk_import.cmake)

project(example_server C CXX ASM)

pico_sdk_init()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../api ${CMAKE_CURRENT_BINARY_DIR}/api_build)

add_executable(example_server
  ${CMAKE_CURRENT_LIST_DIR}/src/example_server.cpp
)
target_include_directories(example_server PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src
)
target_link_libraries(example_server PRIVATE
  nanoipc_ring_buffer
  example_request_reader
  example_response_writer
  pico_stdlib
  hardware_uart
  hardware_gpio
  hardware_irq
)

pico_add_extra_outputs(example_server)
