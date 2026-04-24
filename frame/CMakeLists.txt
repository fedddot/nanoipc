FetchContent_Declare(
    nanocobs
    GIT_REPOSITORY https://github.com/charlesnicholson/nanocobs.git
    GIT_TAG        v0.2.0
)
FetchContent_MakeAvailable(nanocobs)

add_library(read_buffer INTERFACE)
target_include_directories(read_buffer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(ring_buffer INTERFACE)
target_include_directories(ring_buffer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(ring_buffer INTERFACE read_buffer)

add_library(cobs_frame_reader INTERFACE)
target_sources(cobs_frame_reader INTERFACE ${nanocobs_SOURCE_DIR}/cobs.c)
target_include_directories(
    cobs_frame_reader
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${nanocobs_SOURCE_DIR}
)
target_link_libraries(cobs_frame_reader INTERFACE reader read_buffer)

add_library(cobs_frame_writer INTERFACE)
target_sources(cobs_frame_writer INTERFACE ${nanocobs_SOURCE_DIR}/cobs.c)
target_include_directories(
    cobs_frame_writer
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${nanocobs_SOURCE_DIR}
)
target_link_libraries(cobs_frame_writer INTERFACE writer)

if(NANOIPC_TESTS)
  add_subdirectory(tests)
endif()

if(NANOIPC_LINUX_FEATURES)
  add_subdirectory(linux)
endif()