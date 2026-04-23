#include <optional>
#include <vector>

#include "gtest/gtest.h"
#include "pb_message_reader.hpp"
#include "pb_message_writer.hpp"

using namespace nanoipc;

TEST(ut_pb_message, pb_message_reader_ctor_sanity) {
    // WHEN
    RingBuffer<10> ring_buffer;

    // THEN
    ASSERT_NO_THROW(PbMessageReader reader(&ring_buffer));
}

TEST(ut_pb_message, pb_message_writer_ctor_sanity) {
    // WHEN
    const auto dummy_raw_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {

    };

    // THEN
    ASSERT_NO_THROW(PbMessageWriter writer(dummy_raw_data_writer));
}

TEST(ut_pb_message, pb_message_writing_reading_sanity) {
    // GIVEN
    const auto pb_message_data = std::vector<std::uint8_t>{0x01, 0x02, 0x03, 0x00, 0x04};

    // WHEN
    RingBuffer<10> ring_buffer;
    const auto raw_data_writer = [&ring_buffer](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
        for (std::size_t i = 0; i < raw_data_size; ++i) {
            ring_buffer.push_back(raw_data[i]);
        }
    };
    PbMessageReader reader(&ring_buffer);
    PbMessageWriter writer(raw_data_writer);
    std::optional<std::vector<std::uint8_t>> read_pb_message_data(std::nullopt);

    // THEN
    // before writing, there should be no pb_message data to read
    ASSERT_NO_THROW(read_pb_message_data = reader.read());
    ASSERT_FALSE(read_pb_message_data.has_value());

    // after writing, the read pb_message data should be the same as the written pb_message data
    ASSERT_NO_THROW(writer.write(pb_message_data));
    ASSERT_NO_THROW(read_pb_message_data = reader.read());
    ASSERT_TRUE(read_pb_message_data.has_value());
    ASSERT_EQ(read_pb_message_data.value(), pb_message_data);
}