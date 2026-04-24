#ifndef	LINUX_UART_COBS_FRAME_WRITER_HPP
#define	LINUX_UART_COBS_FRAME_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "serialib.h"
#include "cobs_frame_writer.hpp"

namespace nanoipc {
	class LinuxUartCobsFrameWriter: public Writer<std::vector<std::uint8_t>> {
	public:
		LinuxUartCobsFrameWriter(serialib *uart): m_uart(uart) {
			if (!m_uart) {
				throw std::invalid_argument("uart cannot be null");
			}
            m_cobs_frame_writer = std::make_unique<CobsFrameWriter>(
                [this](const std::uint8_t *data, std::size_t length) {
                    unsigned int bytes_written;
                    if (1 != m_uart->writeBytes(reinterpret_cast<const char *>(data), length, &bytes_written) || bytes_written != length) {
                        throw std::runtime_error("Failed to write bytes to UART device");
                    }
                }
            );
		}
		LinuxUartCobsFrameWriter(const LinuxUartCobsFrameWriter&) = delete;
		LinuxUartCobsFrameWriter& operator=(const LinuxUartCobsFrameWriter&) = delete;

		void write(const std::vector<std::uint8_t>& data) const override {
            m_cobs_frame_writer->write(data);
		}
	private:
		serialib *m_uart;
        std::unique_ptr<CobsFrameWriter> m_cobs_frame_writer;
	};
}

#endif // LINUX_UART_COBS_FRAME_WRITER_HPP
