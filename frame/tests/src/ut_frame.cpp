#include "gtest/gtest.h"

#include "frame_reader.hpp"
#include "frame_writer.hpp"
#include "ring_buffer.hpp"

using namespace nanoipc;

TEST(ut_frame_reader, ctor) {
    // WHEN
    RingBuffer<10> ring_buffer;

    // THEN
    ASSERT_NO_THROW(FrameReader reader(&ring_buffer));
}

TEST(ut_frame_writer, ctor) {
    // WHEN
    const auto dummy_raw_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {

    };

    // THEN
    ASSERT_NO_THROW(FrameWriter writer(dummy_raw_data_writer));
}