#ifndef	NANOIPC_UTILS_HPP
#define	NANOIPC_UTILS_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "nanoipc_read_buffer.hpp"

namespace nanoipc {
	/// @brief Attempt to read a single encoded frame from a ReadBuffer.
	///
	/// If a complete encoded frame is available in `read_buffer`, this function
	/// extracts it and returns the encoded bytes. If a complete frame is not
	/// available, an empty `std::optional` is returned and the contents/state of
	/// the provided `ReadBuffer` remain unmodified.
	///
	/// @param read_buffer Pointer to the `ReadBuffer` to read from. Must not be `nullptr`.
	/// @return std::optional<std::vector<std::uint8_t>> Containing the encoded
	///         frame bytes when available, otherwise `std::nullopt`.
	std::optional<std::vector<std::uint8_t>> read_encoded_frame(ReadBuffer *read_buffer);

	/// @brief Encode a raw frame into the wire-format encoded frame.
	///
	/// The returned vector contains the complete encoded frame ready for transmission.
	///
	/// @param frame_data Pointer to the raw frame bytes to encode. May be `nullptr` only when `frame_data_size` is zero.
	/// @param frame_data_size Number of bytes pointed to by `frame_data`.
	/// @return std::vector<std::uint8_t> The encoded frame bytes.
	std::vector<std::uint8_t> encode_frame(const std::uint8_t *frame_data, const std::size_t frame_data_size);

	/// @brief Decode an encoded frame back into its raw payload bytes.
	///
	/// @param frame_data Pointer to the encoded frame bytes.
	/// @param frame_data_size Number of bytes in the encoded frame.
	/// @return std::vector<std::uint8_t> Decoded raw frame bytes.
	std::vector<std::uint8_t> decode_frame(const std::uint8_t *frame_data, const std::size_t frame_data_size);
}

#endif // NANOIPC_UTILS_HPP