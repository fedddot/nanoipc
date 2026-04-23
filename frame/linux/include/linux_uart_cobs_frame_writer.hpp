#ifndef	LINUX_UART_COBS_FRAME_WRITER_HPP
#define	LINUX_UART_COBS_FRAME_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "writer.hpp"
#include "cobs.h"

namespace nanoipc {
	/// @brief Serializes byte vectors into COBS-encoded frames.
	///
	/// CobsFrameWriter encodes data using COBS (Consistent Overhead Byte Stuffing)
	/// and writes the encoded frames via a user-provided callback. It implements the
	/// Writer interface for frame-oriented transport over byte streams such as UART.
	class CobsFrameWriter: public Writer<std::vector<std::uint8_t>> {
	public:
		/// @brief Callback type for writing raw encoded frame data.
		using RawDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		/// @brief Construct a CobsFrameWriter.
		/// @param raw_data_writer Callback function to write the encoded frame data.
		///                        Must not be null/empty.
		/// @throws std::invalid_argument if raw_data_writer is empty/null.
		CobsFrameWriter(const RawDataWriter& raw_data_writer): m_raw_data_writer(raw_data_writer) {
			if (!m_raw_data_writer) {
				throw std::invalid_argument("raw_data_writer cannot be empty");
			}
		}
		CobsFrameWriter(const CobsFrameWriter&) = default;
		CobsFrameWriter& operator=(const CobsFrameWriter&) = default;

		/// @brief Encode and write a byte vector as a COBS frame.
		/// @param data The data to encode and write.
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

#endif // LINUX_UART_COBS_FRAME_WRITER_HPP
