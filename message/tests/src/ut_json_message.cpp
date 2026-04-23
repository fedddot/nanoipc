#include <optional>
#include <string>

#include "gtest/gtest.h"

#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "json_message_reader.hpp"
#include "json_message_writer.hpp"
#include "ring_buffer.hpp"

using namespace nanoipc;

enum: std::size_t { MAX_BUFFER_SIZE = 512 };

TEST(ut_json_message, json_message_reader_ctor_sanity) {
    // WHEN
    RingBuffer<MAX_BUFFER_SIZE> ring_buffer;
    CobsFrameReader frame_reader(&ring_buffer);

    // THEN
    ASSERT_NO_THROW(JsonMessageReader reader(&frame_reader));
}

TEST(ut_json_message, json_message_writer_ctor_sanity) {
    // WHEN
    const auto dummy_raw_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {

    };
    const CobsFrameWriter frame_writer(dummy_raw_data_writer);

    // THEN
    ASSERT_NO_THROW(JsonMessageWriter writer(&frame_writer));
}

TEST(ut_json_message, json_message_reader_null_frame_reader) {
    // THEN
    ASSERT_THROW(JsonMessageReader reader(nullptr), std::invalid_argument);
}

TEST(ut_json_message, json_message_writer_null_frame_writer) {
    // THEN
    ASSERT_THROW(JsonMessageWriter writer(nullptr), std::invalid_argument);
}

TEST(ut_json_message, json_message_writing_reading_sanity) {
    // GIVEN
    Json::Value original_message;
    original_message["id"] = 123;
    original_message["name"] = "test";
    original_message["value"] = 45.67;
    original_message["enabled"] = true;

    // WHEN
    RingBuffer<MAX_BUFFER_SIZE> ring_buffer;
    const auto raw_data_writer = [&ring_buffer](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
        for (std::size_t i = 0; i < raw_data_size; ++i) {
            ring_buffer.push_back(raw_data[i]);
        }
    };
    CobsFrameReader frame_reader(&ring_buffer);
    CobsFrameWriter frame_writer(raw_data_writer);
    JsonMessageReader json_message_reader(&frame_reader);
    JsonMessageWriter json_message_writer(&frame_writer);

    std::optional<Json::Value> read_json_message;

    // THEN
    // before writing, there should be no json_message data to read
    ASSERT_NO_THROW(read_json_message = json_message_reader.read());
    ASSERT_FALSE(read_json_message.has_value());

    // after writing, the read json_message data should be the same as the written json_message data
    ASSERT_NO_THROW(json_message_writer.write(original_message));
    ASSERT_NO_THROW(read_json_message = json_message_reader.read());
    ASSERT_TRUE(read_json_message.has_value());
    ASSERT_EQ(read_json_message.value()["id"].asInt(), original_message["id"].asInt());
    ASSERT_EQ(read_json_message.value()["name"].asString(), original_message["name"].asString());
    ASSERT_EQ(read_json_message.value()["value"].asDouble(), original_message["value"].asDouble());
    ASSERT_EQ(read_json_message.value()["enabled"].asBool(), original_message["enabled"].asBool());
}

TEST(ut_json_message, json_message_complex_structure) {
    // GIVEN
    Json::Value original_message;
    original_message["type"] = "request";
    original_message["params"]["key1"] = "value1";
    original_message["params"]["key2"] = 42;
    Json::Value array;
    array.append("item1");
    array.append("item2");
    array.append("item3");
    original_message["items"] = array;

    // WHEN
    RingBuffer<MAX_BUFFER_SIZE> ring_buffer;
    const auto raw_data_writer = [&ring_buffer](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
        for (std::size_t i = 0; i < raw_data_size; ++i) {
            ring_buffer.push_back(raw_data[i]);
        }
    };
    CobsFrameReader frame_reader(&ring_buffer);
    CobsFrameWriter frame_writer(raw_data_writer);
    JsonMessageReader json_message_reader(&frame_reader);
    JsonMessageWriter json_message_writer(&frame_writer);

    std::optional<Json::Value> read_json_message;

    // THEN
    ASSERT_NO_THROW(json_message_writer.write(original_message));
    ASSERT_NO_THROW(read_json_message = json_message_reader.read());
    ASSERT_TRUE(read_json_message.has_value());
    ASSERT_EQ(read_json_message.value()["type"].asString(), "request");
    ASSERT_EQ(read_json_message.value()["params"]["key1"].asString(), "value1");
    ASSERT_EQ(read_json_message.value()["params"]["key2"].asInt(), 42);
    ASSERT_EQ(read_json_message.value()["items"].size(), 3);
    ASSERT_EQ(read_json_message.value()["items"][0].asString(), "item1");
    ASSERT_EQ(read_json_message.value()["items"][1].asString(), "item2");
    ASSERT_EQ(read_json_message.value()["items"][2].asString(), "item3");
}

TEST(ut_json_message, json_message_invalid_json_read) {
    // GIVEN - manually inject invalid JSON into the ring buffer
    RingBuffer<MAX_BUFFER_SIZE> ring_buffer;
    std::string invalid_json = "{invalid json";
    std::vector<std::uint8_t> invalid_bytes(invalid_json.begin(), invalid_json.end());

    const auto raw_data_writer = [&ring_buffer](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
        for (std::size_t i = 0; i < raw_data_size; ++i) {
            ring_buffer.push_back(raw_data[i]);
        }
    };

    CobsFrameReader frame_reader(&ring_buffer);
    CobsFrameWriter frame_writer(raw_data_writer);
    JsonMessageReader json_message_reader(&frame_reader);
    JsonMessageWriter json_message_writer(&frame_writer);

    // Create a mock reader that returns invalid JSON
    class MockFrameReader: public Reader<std::vector<std::uint8_t>> {
    public:
        std::optional<std::vector<std::uint8_t>> read() override {
            std::string invalid = "{bad json";
            return std::make_optional(std::vector<std::uint8_t>(invalid.begin(), invalid.end()));
        }
    };

    MockFrameReader mock_reader;
    JsonMessageReader json_reader_with_mock(&mock_reader);

    // THEN - reading invalid JSON should throw
    ASSERT_THROW(json_reader_with_mock.read(), std::runtime_error);
}
