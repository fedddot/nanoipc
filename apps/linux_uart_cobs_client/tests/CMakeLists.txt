add_executable(
    linux_uart_cobs_client_it
    ${CMAKE_CURRENT_LIST_DIR}/src/it_linux_uart_cobs_client.cpp
)

target_link_libraries(
    linux_uart_cobs_client_it
    PRIVATE
    virtual_uart
    cobs_frame_reader
    cobs_frame_writer
    ring_buffer
    gtest
    gtest_main
)

# Ensure the app binary is built before the integration test runs.
add_dependencies(linux_uart_cobs_client_it linux_uart_cobs_client)

# Inject the path to the built app binary at compile time.
target_compile_definitions(
    linux_uart_cobs_client_it
    PRIVATE
    APP_BINARY="$<TARGET_FILE:linux_uart_cobs_client>"
)

add_test(NAME linux_uart_cobs_client_it COMMAND linux_uart_cobs_client_it)
