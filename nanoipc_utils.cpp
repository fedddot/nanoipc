#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "nanoipc_read_buffer.hpp"
#include "cobs.h"

#include "nanoipc_utils.hpp"

using namespace nanoipc;

std::optional<std::size_t> nanoipc::find_delimiter(const ReadBuffer& read_buffer) {
    for (auto i = std::size_t(0); i < read_buffer.size(); ++i) {
        if (read_buffer.get(i) == COBS_FRAME_DELIMITER) {
            return i;
        }
    }
    return std::nullopt;
}

inline std::optional<std::vector<std::uint8_t>> nanoipc::read_encoded_frame(ReadBuffer *read_buffer) {
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

inline std::optional<std::vector<std::uint8_t>> nanoipc::read_frame(ReadBuffer *read_buffer) {
    const auto encoded_frame_data = read_encoded_frame(read_buffer);
    if (!encoded_frame_data.has_value()) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> decoded_frame_data;
    std::size_t decoded_frame_data_size = 0;
    decoded_frame_data.reserve(encoded_frame_data->size());
    const auto decode_result = cobs_decode(encoded_frame_data->data(), encoded_frame_data->size(), decoded_frame_data.data(), decoded_frame_data.capacity(), &decoded_frame_data_size);
    if (decode_result != COBS_RET_SUCCESS) {
        throw std::runtime_error("failed to decode COBS frame");
    }
    decoded_frame_data.resize(decoded_frame_data_size);
    return std::make_optional(decoded_frame_data);
}