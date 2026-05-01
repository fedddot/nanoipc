cmake_minimum_required(VERSION 3.16)

project(nanoipc C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(reader INTERFACE)
target_include_directories(reader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(writer INTERFACE)
target_include_directories(writer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

include(FetchContent)

option(NANOIPC_LINUX_FEATURES "Build with Linux-specific features" ON)
option(NANOIPC_TESTS "Build the project's test suite" ON)
if(NANOIPC_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG		 v1.17.0
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(testing)
endif()

add_subdirectory(apps)
add_subdirectory(frame)
add_subdirectory(message)
