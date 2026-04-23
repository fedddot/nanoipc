add_executable(
    utils_tests
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_uart_connection.cpp
)

target_link_libraries(
    utils_tests
    PRIVATE
    uart_connection
    gtest
    gtest_main
)

add_test(NAME utils_tests COMMAND utils_tests)

add_executable(json_client ${CMAKE_CURRENT_LIST_DIR}/src/json_client.cpp)
target_link_libraries(
    json_client
    PRIVATE
    cobs_frame_reader
    cobs_frame_writer
    json_message_reader
    json_message_writer
    uart_connection
    ring_buffer
)