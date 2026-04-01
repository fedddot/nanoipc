#ifndef	NANOIPC_UTILS_HPP
#define	NANOIPC_UTILS_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "nanoipc_read_buffer.hpp"

namespace nanoipc {
	inline std::optional<std::size_t> find_delimiter(const ReadBuffer& read_buffer);
	inline std::optional<std::vector<std::uint8_t>> read_encoded_frame(ReadBuffer *read_buffer);
	inline std::optional<std::vector<std::uint8_t>> read_frame(ReadBuffer *read_buffer);
}

#endif // NANOIPC_UTILS_HPP