#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "nanoipc_read_buffer.hpp"
#include "cobs.h"

#include "nanoipc_utils.hpp"

using namespace nanoipc;





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

