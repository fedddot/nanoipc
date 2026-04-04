#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "nanoipc_read_buffer.hpp"
#include "cobs.h"

#include "nanoipc_utils.hpp"

using namespace nanoipc;

static std::optional<std::size_t> find_delimiter(const ReadBuffer& read_buffer) {
    for (auto i = std::size_t(0); i < read_buffer.size(); ++i) {
        if (read_buffer.get(i) == COBS_FRAME_DELIMITER) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<std::vector<std::uint8_t>> nanoipc::read_encoded_frame(ReadBuffer *read_buffer) {
    const auto delimiter_index = find_delimiter(*read_buffer);
    if (!delimiter_index.has_value()) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> encoded_frame_data;
    encoded_frame_data.reserve(delimiter_index.value() + 1);
    for (auto i = std::size_t(0); i <= delimiter_index.value(); ++i) {
        encoded_frame_data.push_back(read_buffer->pop_front());
    }
    return std::make_optional(encoded_frame_data);
}

std::vector<std::uint8_t> nanoipc::encode_frame(const std::uint8_t *frame_data, const std::size_t frame_data_size) {
    enum: std::size_t { GROW_FACTOR = 2 };
    std::vector<std::uint8_t> encoded_frame_data;
    std::size_t receiver_data_size = frame_data_size;
    std::size_t encoded_frame_data_size = 0;

    while (true) {
        encoded_frame_data.resize(receiver_data_size);
        const auto encode_result = cobs_encode(frame_data, frame_data_size, encoded_frame_data.data(), encoded_frame_data.size(), &encoded_frame_data_size);
        if (encode_result == COBS_RET_SUCCESS) {
            break;
        } else if (encode_result == COBS_RET_ERR_EXHAUSTED) {
            receiver_data_size *= GROW_FACTOR;
        } else {
            throw std::runtime_error("failed to encode COBS frame");
        }
    }
    encoded_frame_data.resize(encoded_frame_data_size);
    return encoded_frame_data;
}

std::vector<std::uint8_t> nanoipc::decode_frame(const std::uint8_t *frame_data, const std::size_t frame_data_size) {
    std::vector<std::uint8_t> decoded_frame_data(frame_data_size);
    std::size_t decoded_frame_data_size = 0;
    const auto decode_result = cobs_decode(frame_data, frame_data_size, decoded_frame_data.data(), decoded_frame_data.size(), &decoded_frame_data_size);
    if (decode_result != COBS_RET_SUCCESS) {
        throw std::runtime_error("failed to decode COBS frame");
    }
    decoded_frame_data.resize(decoded_frame_data_size);
    return decoded_frame_data;
}