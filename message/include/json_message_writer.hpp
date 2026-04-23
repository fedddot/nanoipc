#ifndef JSON_MESSAGE_WRITER_HPP
#define JSON_MESSAGE_WRITER_HPP

#include "json/writer.h"
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "json/value.h"
#include "writer.hpp"

namespace nanoipc {
    /// @brief Serializes JSON messages into frame data.
    ///
    /// JsonMessageWriter encodes a JSON message (Json::Value) into a string representation
    /// and writes the encoded data as a byte vector via a Writer<std::vector<std::uint8_t>>.
    /// This is typically used with a frame writer to handle the transport framing.
    class JsonMessageWriter: public Writer<Json::Value> {
    public:
        /// @brief Construct a JsonMessageWriter.
        /// @param frame_writer Pointer to a Writer<std::vector<std::uint8_t>> that writes encoded data.
        ///                     Must not be null.
        /// @throws std::invalid_argument if frame_writer is null.
        JsonMessageWriter(const Writer<std::vector<std::uint8_t>> *frame_writer)
            : m_frame_writer(frame_writer) {
            if (!m_frame_writer) {
                throw std::invalid_argument("invalid arguments in JsonMessageWriter ctor");
            }
        }
        JsonMessageWriter(const JsonMessageWriter&) = default;
        JsonMessageWriter& operator=(const JsonMessageWriter&) = default;

        /// @brief Encode and write a JSON message.
        /// @param data The message to encode and write.
        void write(const Json::Value& data) const override {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            std::string json_str = Json::writeString(builder, data);

            std::vector<std::uint8_t> bytes(json_str.begin(), json_str.end());
            m_frame_writer->write(bytes);
        }

    private:
        const Writer<std::vector<std::uint8_t>> *m_frame_writer;
    };
}

#endif // JSON_MESSAGE_WRITER_HPP
