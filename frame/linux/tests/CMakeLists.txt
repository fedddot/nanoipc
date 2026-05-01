add_executable(
    linux_uart_frame_tests
    src/ut_linux_uart_cobs_frame.cpp
)

target_link_libraries(
    linux_uart_frame_tests
    PRIVATE
    virtual_uart
    linux_uart_cobs_frame_reader
    linux_uart_cobs_frame_writer
)

add_test(NAME linux_uart_frame_tests COMMAND linux_uart_frame_tests)
