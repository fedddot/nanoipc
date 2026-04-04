set(NANOPB_SRC_ROOT_FOLDER ${nanopb_SOURCE_DIR})
list(APPEND CMAKE_MODULE_PATH ${nanopb_SOURCE_DIR}/extra)
find_package(Nanopb REQUIRED)

set(NANOPB_GENERATE_CPP_STANDALONE FALSE)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS
  ${CMAKE_CURRENT_LIST_DIR}/src/test.proto
)

add_executable(
  nanopb_utils_tests
  ${CMAKE_CURRENT_LIST_DIR}/src/ut_nanopb_serializer.cpp
  ${PROTO_SRCS}
)

target_link_libraries(
  nanopb_utils_tests
  PRIVATE
  nanopb_serializer
  gtest
  gtest_main
)

target_include_directories(
  nanopb_utils_tests
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME nanopb_utils_tests COMMAND nanopb_utils_tests)
