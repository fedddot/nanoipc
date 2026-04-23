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

option(BUILD_TESTS "Build the project's test suite" ON)
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG		 v1.17.0
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

add_subdirectory(frame)
add_subdirectory(message)

# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nanopb_utils)



# add_library(nanoipc_reader INTERFACE)
# target_include_directories(nanoipc_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR})
# target_link_libraries(nanoipc_reader INTERFACE nanoipc_utils nanoipc_read_buffer)

# add_library(nanoipc_writer INTERFACE)
# target_include_directories(nanoipc_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
# target_link_libraries(nanoipc_writer INTERFACE nanoipc_utils)

# if(BUILD_TESTS)
#   add_subdirectory(tests)
# endif()