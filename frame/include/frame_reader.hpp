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

		std::optional<std::vector<std::uint8_t>> read() {
			const auto delimiter_index = find_delimiter(*m_read_buffer);
			if (!delimiter_index.has_value()) {
				return std::nullopt;
			}
			std::vector<std::uint8_t> encoded_frame_data;
			encoded_frame_data.reserve(delimiter_index.value() + 1);
			for (auto i = std::size_t(0); i <= delimiter_index.value(); ++i) {
				encoded_frame_data.push_back(m_read_buffer->pop_front());
			}
			return std::make_optional(encoded_frame_data);
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
	};
}

#endif // FRAME_READER_HPP