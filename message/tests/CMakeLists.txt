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
    message_tests
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_pb_message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_json_message.cpp
)

target_link_libraries(
    message_tests
    PRIVATE
    pb_message_reader
    pb_message_writer
    json_message_reader
    json_message_writer
    test_api

    cobs_frame_reader
    cobs_frame_writer
    ring_buffer

    gtest
    gtest_main
)

target_include_directories(
    message_tests
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME message_tests COMMAND message_tests)
