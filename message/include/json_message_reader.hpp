#ifndef JSON_MESSAGE_READER_HPP
#define JSON_MESSAGE_READER_HPP

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "json/reader.h"
#include "json/value.h"
#include "reader.hpp"

namespace nanoipc {
    /// @brief Deserializes JSON messages from frame data.
    ///
    /// JsonMessageReader reads encoded frame data using a Reader<std::vector<std::uint8_t>>,
    /// decodes it as JSON, and provides the deserialized message as a Json::Value
    /// via the Reader interface.
    class JsonMessageReader: public Reader<Json::Value> {
    public:
        /// @brief Construct a JsonMessageReader.
        /// @param frame_reader Pointer to a Reader<std::vector<std::uint8_t>> that provides encoded frame data.
        ///                     Must not be null.
        /// @throws std::invalid_argument if frame_reader is null.
        JsonMessageReader(Reader<std::vector<std::uint8_t>> *frame_reader)
            : m_frame_reader(frame_reader) {
            if (!m_frame_reader) {
                throw std::invalid_argument("invalid arguments in JsonMessageReader ctor");
            }
        }
        JsonMessageReader(const JsonMessageReader&) = default;
        JsonMessageReader& operator=(const JsonMessageReader&) = default;

        /// @brief Read and decode the next JSON message.
        /// @return An optional containing the decoded message, or std::nullopt if no frame data is available.
        std::optional<Json::Value> read() override {
            auto frame_data = m_frame_reader->read();
            if (!frame_data.has_value()) {
                return std::nullopt;
            }
            std::string json_str(frame_data->begin(), frame_data->end());
            frame_data.reset();
            Json::Value root;
            std::string errs;
            Json::CharReaderBuilder builder;
            builder["collectComments"] = false;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

            if (!reader->parse(json_str.data(), json_str.data() + json_str.size(), &root, &errs)) {
                throw std::runtime_error("failed to decode JSON: " + errs);
            }

            return std::make_optional(std::move(root));
        }

    private:
        Reader<std::vector<std::uint8_t>> *m_frame_reader;
    };
}

#endif // JSON_MESSAGE_READER_HPP
