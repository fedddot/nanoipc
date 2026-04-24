#ifndef	LINUX_UART_COBS_FRAME_READER_HPP
#define	LINUX_UART_COBS_FRAME_READER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "serialib.h"

#include "reader.hpp"
#include "ring_buffer.hpp"
#include "cobs_frame_reader.hpp"

namespace nanoipc {
    /// @brief Reads COBS-framed byte vectors from a Linux UART device.
    ///
    /// Drains all available bytes from the UART on each call to read(),
    /// buffers them in an internal ring buffer, and delegates COBS
    /// frame reassembly to CobsFrameReader.
    ///
    /// @tparam N Capacity of the internal ring buffer in bytes.
    template <std::size_t N>
	class LinuxUartCobsFrameReader: public Reader<std::vector<std::uint8_t>> {
	public:
        /// @brief Constructs a reader bound to the given UART device.
        /// @param uart Pointer to an open serialib instance. Must not be null.
		LinuxUartCobsFrameReader(serialib *uart): m_uart(uart), m_read_buffer() {
			if (!m_uart) {
				throw std::invalid_argument("uart cannot be null");
			}
            m_cobs_frame_reader = std::make_unique<CobsFrameReader>(&m_read_buffer);
		}
		LinuxUartCobsFrameReader(const LinuxUartCobsFrameReader&) = delete;
		LinuxUartCobsFrameReader& operator=(const LinuxUartCobsFrameReader&) = delete;

        /// @brief Reads one complete COBS frame, if available.
        /// @return The decoded frame payload, or std::nullopt if no complete frame is ready yet.
		std::optional<std::vector<std::uint8_t>> read() override {
            while (0 < m_uart->available()) {
                char byte;
                if (1 != m_uart->readChar(&byte)) {
                    throw std::runtime_error("Failed to read byte from UART device");
                }
                m_read_buffer.push_back(static_cast<std::uint8_t>(byte));
            }
            return m_cobs_frame_reader->read();
		}
	private:
		serialib *m_uart;
        RingBuffer<N> m_read_buffer;
        std::unique_ptr<CobsFrameReader> m_cobs_frame_reader;
	};
}

#endif // LINUX_UART_COBS_FRAME_READER_HPP