#ifndef	NANOIPC_UTILS_HPP
#define	NANOIPC_UTILS_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "nanoipc_read_buffer.hpp"

namespace nanoipc {
	std::optional<std::vector<std::uint8_t>> read_encoded_frame(ReadBuffer *read_buffer);
	std::vector<std::uint8_t> encode_frame(const std::uint8_t *frame_data, const std::size_t frame_data_size);
	std::vector<std::uint8_t> decode_frame(const std::uint8_t *frame_data, const std::size_t frame_data_size);
}

#endif // NANOIPC_UTILS_HPP