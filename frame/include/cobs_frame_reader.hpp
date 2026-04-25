#ifndef	COBS_FRAME_READER_HPP
#define	COBS_FRAME_READER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "read_buffer.hpp"
#include "reader.hpp"
#include "cobs.h"

namespace nanoipc {
	/// @brief Deserializes COBS-encoded frames into byte vectors.
	///
	/// CobsFrameReader reads COBS (Consistent Overhead Byte Stuffing) encoded
	/// frames from a ReadBuffer, decodes them, and provides the decoded data
	/// as std::vector<std::uint8_t>. It implements the Reader interface for
	/// frame-oriented transport over byte streams such as UART.
	class CobsFrameReader: public Reader<std::vector<std::uint8_t>> {
	public:
		/// @brief Construct a CobsFrameReader.
		/// @param read_buffer Pointer to the ReadBuffer from which encoded frames are read.
		///                    Must not be null.
		/// @throws std::invalid_argument if read_buffer is null.
		CobsFrameReader(ReadBuffer *read_buffer): m_read_buffer(read_buffer) {
			if (!m_read_buffer) {
				throw std::invalid_argument("read_buffer cannot be null");
			}
		}
		CobsFrameReader(const CobsFrameReader&) = default;
		CobsFrameReader& operator=(const CobsFrameReader&) = default;

		/// @brief Read and decode the next complete COBS frame.
		/// @return An optional containing the decoded frame data, or std::nullopt if no complete frame is available.
		std::optional<std::vector<std::uint8_t>> read() override {
            auto encoded_frame = read_encoded_frame(m_read_buffer);
            if (!encoded_frame.has_value()) {
                return std::nullopt;
            }
            return std::make_optional(std::move(decode_frame(encoded_frame.value())));
		}

	private:
		ReadBuffer *m_read_buffer;
		static std::optional<std::size_t> find_delimiter(const ReadBuffer& read_buffer) {
			for (auto i = std::size_t(0); i < read_buffer.size(); ++i) {
				if (read_buffer.get(i) == COBS_FRAME_DELIMITER) {
					return i;
				}
			}
			return std::nullopt;
		}
        static std::optional<std::vector<std::uint8_t>> read_encoded_frame(ReadBuffer *read_buffer) {
            const auto delimiter_index = find_delimiter(*read_buffer);
			if (!delimiter_index.has_value()) {
				return std::nullopt;
			}
			std::vector<std::uint8_t> encoded_frame_data;
			encoded_frame_data.reserve(delimiter_index.value() + 1);
			for (auto i = std::size_t(0); i <= delimiter_index.value(); ++i) {
				encoded_frame_data.push_back(read_buffer->pop_front());
			}
			return std::make_optional(std::move(encoded_frame_data));
        }
        static std::vector<std::uint8_t> decode_frame(const std::vector<std::uint8_t>& frame_data) {
            std::vector<std::uint8_t> decoded_frame_data(frame_data.size());
            std::size_t decoded_frame_data_size = 0;
            const auto decode_result = cobs_decode(frame_data.data(), frame_data.size(), decoded_frame_data.data(), decoded_frame_data.size(), &decoded_frame_data_size);
            if (decode_result != COBS_RET_SUCCESS) {
                throw std::runtime_error("failed to decode COBS frame");
            }
            decoded_frame_data.resize(decoded_frame_data_size);
            return std::move(decoded_frame_data);
        }
	};
}

#endif // COBS_FRAME_READER_HPP