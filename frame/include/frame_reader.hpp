#ifndef	FRAME_READER_HPP
#define	FRAME_READER_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <vector>

#include "read_buffer.hpp"
#include "reader.hpp"
#include "cobs.h"

namespace nanoipc {
	class FrameReader: public Reader<std::vector<std::uint8_t>> {
	public:
		FrameReader(
			ReadBuffer *read_buffer
		): m_read_buffer(read_buffer) {
			if (!m_read_buffer) {
				throw std::invalid_argument("read_buffer cannot be null");
			}
		}
		FrameReader(const FrameReader&) = default;
		FrameReader& operator=(const FrameReader&) = default;
		virtual ~FrameReader() noexcept = default;

		std::optional<std::vector<std::uint8_t>> read() override {
            const auto encoded_frame = read_encoded_frame(m_read_buffer);
            if (!encoded_frame.has_value()) {
                return std::nullopt;
            }
            return std::make_optional(decode_frame(encoded_frame.value()));
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
			return std::make_optional(encoded_frame_data);
        }
        static std::vector<std::uint8_t> decode_frame(const std::vector<std::uint8_t>& frame_data) {
            std::vector<std::uint8_t> decoded_frame_data(frame_data.size());
            std::size_t decoded_frame_data_size = 0;
            const auto decode_result = cobs_decode(frame_data.data(), frame_data.size(), decoded_frame_data.data(), decoded_frame_data.size(), &decoded_frame_data_size);
            if (decode_result != COBS_RET_SUCCESS) {
                throw std::runtime_error("failed to decode COBS frame");
            }
            decoded_frame_data.resize(decoded_frame_data_size);
            return decoded_frame_data;
        }
	};
}

#endif // FRAME_READER_HPP