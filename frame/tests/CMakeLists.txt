add_executable(
    frame_tests
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_frame_reader.cpp
)

target_link_libraries(
    frame_tests
    PRIVATE
    frame_reader
    ring_buffer
    gtest
    gtest_main
)

add_test(NAME frame_tests COMMAND frame_tests)
