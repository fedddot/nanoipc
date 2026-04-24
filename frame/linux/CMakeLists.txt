FetchContent_Declare(
    serialib
    GIT_REPOSITORY https://github.com/imabot2/serialib.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(serialib)

# Create an interface library for serialib since it doesn't have CMakeLists.txt
add_library(serialib_interface INTERFACE)
target_include_directories(serialib_interface INTERFACE ${serialib_SOURCE_DIR}/lib)
target_sources(serialib_interface INTERFACE ${serialib_SOURCE_DIR}/lib/serialib.cpp)

add_library(linux_uart_cobs_frame_reader INTERFACE)
target_include_directories(linux_uart_cobs_frame_reader INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    linux_uart_cobs_frame_reader
    INTERFACE
    cobs_frame_reader
    reader
    ring_buffer
    serialib_interface
)

add_library(linux_uart_cobs_frame_writer INTERFACE)
target_include_directories(linux_uart_cobs_frame_writer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(
    linux_uart_cobs_frame_writer
    INTERFACE
    writer
    cobs_frame_writer
    serialib_interface
)