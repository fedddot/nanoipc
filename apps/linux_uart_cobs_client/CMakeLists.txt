FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.4.1
)
FetchContent_MakeAvailable(cli11)

add_executable(linux_uart_cobs_client ${CMAKE_CURRENT_LIST_DIR}/src/linux_uart_cobs_client.cpp)
target_link_libraries(
    linux_uart_cobs_client
    PRIVATE
    CLI11::CLI11
    linux_uart_cobs_frame_reader
    linux_uart_cobs_frame_writer
    serialib_interface
)

if(NANOIPC_TESTS)
    add_subdirectory(tests)
endif()
