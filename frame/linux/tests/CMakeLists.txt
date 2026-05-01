add_executable(
    linux_uart_frame_tests
    src/ut_linux_uart_cobs_frame.cpp
)

target_link_libraries(
    linux_uart_frame_tests
    PRIVATE
    cobs_frame_reader
    linux_uart_cobs_frame_reader
    linux_uart_cobs_frame_writer
    virtual_uart

    gtest
    gtest_main
)

add_test(NAME linux_uart_frame_tests COMMAND linux_uart_frame_tests)
