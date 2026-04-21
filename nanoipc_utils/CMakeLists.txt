FetchContent_Declare(
  nanocobs
  GIT_REPOSITORY https://github.com/charlesnicholson/nanocobs.git
  GIT_TAG        v0.2.0
)
FetchContent_MakeAvailable(nanocobs)

add_library(nanoipc_utils INTERFACE)
target_sources(nanoipc_utils INTERFACE ${CMAKE_CURRENT_LIST_DIR}/nanoipc_utils.cpp ${nanocobs_SOURCE_DIR}/cobs.c)
target_include_directories(nanoipc_utils INTERFACE ${CMAKE_CURRENT_LIST_DIR} ${nanocobs_SOURCE_DIR})
target_link_libraries(nanoipc_utils INTERFACE nanoipc_read_buffer)

add_library(nanoipc_ring_buffer INTERFACE)
target_include_directories(nanoipc_ring_buffer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
