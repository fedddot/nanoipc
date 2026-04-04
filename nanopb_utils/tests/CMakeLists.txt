set(TEST_GENERATED_API_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
	OUTPUT ${TEST_GENERATED_API_DIR}/test.pb.c ${TEST_GENERATED_API_DIR}/test.pb.h
	DEPENDS ${CMAKE_CURRENT_LIST_DIR}/src/test.proto
	COMMAND ${Python_EXECUTABLE} ${nanopb_SOURCE_DIR}/generator/nanopb_generator.py -I ${CMAKE_CURRENT_LIST_DIR}/src -D ${TEST_GENERATED_API_DIR} test.proto
)

add_custom_target(
	generate_test_api
	DEPENDS ${TEST_GENERATED_API_DIR}/test.pb.c ${TEST_GENERATED_API_DIR}/test.pb.h
)

add_library(test_api INTERFACE)
add_dependencies(test_api generate_test_api)
target_sources(test_api INTERFACE ${TEST_GENERATED_API_DIR}/test.pb.c)
target_include_directories(test_api INTERFACE ${TEST_GENERATED_API_DIR})

add_executable(
  nanopb_utils_tests
  ${CMAKE_CURRENT_LIST_DIR}/src/ut_nanopb_serializer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/ut_nanopb_parser.cpp
)

target_link_libraries(
  nanopb_utils_tests
  PRIVATE
  nanopb_serializer
  nanopb_parser
  test_api
  gtest
  gtest_main
)

target_include_directories(
  nanopb_utils_tests
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME nanopb_utils_tests COMMAND nanopb_utils_tests)
