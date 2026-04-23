#ifndef	FRAME_WRITER_HPP
#define	FRAME_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "writer.hpp"
#include "cobs.h"

namespace nanoipc {
	class FrameWriter: public Writer<std::vector<std::uint8_t>> {
	public:
		using RawDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;
		FrameWriter(const RawDataWriter& raw_data_writer): m_raw_data_writer(raw_data_writer) {
			if (!m_raw_data_writer) {
				throw std::invalid_argument("raw_data_writer cannot be empty");
			}
		}
		FrameWriter(const FrameWriter&) = default;
		FrameWriter& operator=(const FrameWriter&) = default;

		void write(const std::vector<std::uint8_t>& data) const override {
			const auto encoded_message = encode_frame(data);
			m_raw_data_writer(encoded_message.data(), encoded_message.size());
		}

	private:
		RawDataWriter m_raw_data_writer;
        static std::vector<std::uint8_t> encode_frame(const std::vector<std::uint8_t>& data) {
            enum: std::size_t { SIZE_INCREMENT = 2 };
            std::vector<std::uint8_t> encoded_frame_data;
            std::size_t receiver_data_size = data.size() + SIZE_INCREMENT;
            std::size_t encoded_frame_data_size = 0;

            while (true) {
                encoded_frame_data.resize(receiver_data_size);
                const auto encode_result = cobs_encode(data.data(), data.size(), encoded_frame_data.data(), encoded_frame_data.size(), &encoded_frame_data_size);
                if (encode_result == COBS_RET_SUCCESS) {
                    break;
                } else if (encode_result == COBS_RET_ERR_EXHAUSTED) {
                    receiver_data_size += SIZE_INCREMENT;
                } else {
                    throw std::runtime_error("failed to encode COBS frame");
                }
            }
            encoded_frame_data.resize(encoded_frame_data_size);
            return encoded_frame_data;
        }
	};
}

#endif // FRAME_WRITER_HPP
