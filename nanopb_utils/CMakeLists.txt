FetchContent_Declare(
  nanopb
  GIT_REPOSITORY https://github.com/nanopb/nanopb.git
  GIT_TAG        nanopb-0.4.9
)
FetchContent_MakeAvailable(nanopb)

add_library(nanopb_common INTERFACE)
target_sources(nanopb_common INTERFACE ${nanopb_SOURCE_DIR}/pb_common.c)
target_include_directories(nanopb_common INTERFACE ${nanopb_SOURCE_DIR})

add_library(nanopb_encode INTERFACE)
target_sources(nanopb_encode INTERFACE ${nanopb_SOURCE_DIR}/pb_encode.c)
target_link_libraries(nanopb_encode INTERFACE nanopb_common)

add_library(nanopb_decode INTERFACE)
target_sources(nanopb_decode INTERFACE ${nanopb_SOURCE_DIR}/pb_decode.c)
target_link_libraries(nanopb_decode INTERFACE nanopb_common)

add_library(nanopb_serializer INTERFACE)
target_include_directories(nanopb_serializer INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanopb_serializer INTERFACE nanopb_encode)

add_library(nanopb_parser INTERFACE)
target_include_directories(nanopb_parser INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(nanopb_parser INTERFACE nanopb_decode)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()