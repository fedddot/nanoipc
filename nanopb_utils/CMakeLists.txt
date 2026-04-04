FetchContent_Declare(
  nanopb
  GIT_REPOSITORY https://github.com/nanopb/nanopb.git
  GIT_TAG        nanopb-0.4.9
)
FetchContent_MakeAvailable(nanopb)

add_library(nanopb_encode INTERFACE)
target_sources(nanopb_encode INTERFACE ${nanopb_SOURCE_DIR}/pb_encode.c ${nanopb_SOURCE_DIR}/pb_common.c)
target_include_directories(nanopb_encode INTERFACE ${nanopb_SOURCE_DIR})

add_library(nanopb_serializer INTERFACE)
target_include_directories(nanopb_serializer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanopb_serializer INTERFACE nanopb_encode)