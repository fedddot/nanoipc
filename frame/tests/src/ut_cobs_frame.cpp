#include "gtest/gtest.h"
#include <optional>
#include <vector>

#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "ring_buffer.hpp"

using namespace nanoipc;

TEST(ut_cobs_frame, cobs_frame_reader_ctor_sanity) {
    // WHEN
    RingBuffer<10> ring_buffer;

    // THEN
    ASSERT_NO_THROW(FrameReader reader(&ring_buffer));
}

TEST(ut_cobs_frame, cobs_frame_writer_ctor_sanity) {
    // WHEN
    const auto dummy_raw_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {

    };

    // THEN
    ASSERT_NO_THROW(FrameWriter writer(dummy_raw_data_writer));
}

TEST(ut_cobs_frame, frame_writing_reading_sanity) {
    // GIVEN
    const auto frame_data = std::vector<std::uint8_t>{0x01, 0x02, 0x03, 0x00, 0x04};

    // WHEN
    RingBuffer<10> ring_buffer;
    const auto raw_data_writer = [&ring_buffer](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
        for (std::size_t i = 0; i < raw_data_size; ++i) {
            ring_buffer.push_back(raw_data[i]);
        }
    };
    FrameReader reader(&ring_buffer);
    FrameWriter writer(raw_data_writer);
    std::optional<std::vector<std::uint8_t>> read_frame_data(std::nullopt);

    // THEN
    // before writing, there should be no frame data to read
    ASSERT_NO_THROW(read_frame_data = reader.read());
    ASSERT_FALSE(read_frame_data.has_value());

    // after writing, the read frame data should be the same as the written frame data
    ASSERT_NO_THROW(writer.write(frame_data));
    ASSERT_NO_THROW(read_frame_data = reader.read());
    ASSERT_TRUE(read_frame_data.has_value());
    ASSERT_EQ(read_frame_data.value(), frame_data);
}