FetchContent_Declare(
    nanocobs
    GIT_REPOSITORY https://github.com/charlesnicholson/nanocobs.git
    GIT_TAG        v0.2.0
)
FetchContent_MakeAvailable(nanocobs)

add_library(read_buffer INTERFACE)
target_include_directories(read_buffer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(frame_reader INTERFACE)
target_sources(frame_reader INTERFACE ${nanocobs_SOURCE_DIR}/cobs.c)
target_include_directories(
    frame_reader
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${nanocobs_SOURCE_DIR}
)
target_link_libraries(frame_reader INTERFACE read_buffer)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()