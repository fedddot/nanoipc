cmake_minimum_required(VERSION 3.16)
project(example_server C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Path to the nanoipc repository root ───────────────────────────────────────
get_filename_component(NANOIPC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# ── Pull in the root nanoipc project (provides nanoipc_reader, nanoipc_writer,
#    nanopb_serializer, nanopb_parser and all their transitive deps) ────────────
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${NANOIPC_ROOT}" nanoipc)

# Use the shared example proto project
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../api")

# ── Server executable ─────────────────────────────────────────────────────────
add_executable(example_server
  ${CMAKE_CURRENT_LIST_DIR}/src/example_server.cpp
)

target_include_directories(example_server PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(example_server PRIVATE
  nanoipc_reader
  nanoipc_writer
  nanopb_serializer
  nanopb_parser
  example_api
)

# ── STM32 CMSIS headers (STM32F1 device + ARM Cortex-M3 core) ────────────────
FetchContent_Declare(
  cmsis_device_f1
  GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis_device_f1.git
  GIT_TAG        v4.3.4
  GIT_SHALLOW    TRUE
)
FetchContent_Declare(
  cmsis_core
  GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis_core.git
  GIT_TAG        v5.4.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cmsis_device_f1 cmsis_core)

add_library(stm32_cmsis INTERFACE)
target_include_directories(stm32_cmsis INTERFACE
  "${cmsis_device_f1_SOURCE_DIR}/Include"
  "${cmsis_core_SOURCE_DIR}/Include"
)

target_sources(example_server PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/src/startup_stm32f103.c"
)
target_link_libraries(example_server PRIVATE stm32_cmsis)

# Pull in the stm32_generate_bin macro from the toolchain helper.
include(${CMAKE_CURRENT_LIST_DIR}/stm32-bluepill-toolchain.cmake OPTIONAL)
stm32_generate_bin(example_server
  "${CMAKE_CURRENT_LIST_DIR}/STM32F103C8Tx_FLASH.ld"
)
