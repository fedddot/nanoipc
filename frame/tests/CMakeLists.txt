add_executable(
    frame_tests
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_cobs_frame.cpp
)

target_link_libraries(
    frame_tests
    PRIVATE
    cobs_frame_reader
    cobs_frame_writer
    ring_buffer
    gtest
    gtest_main
)

add_test(NAME frame_tests COMMAND frame_tests)
