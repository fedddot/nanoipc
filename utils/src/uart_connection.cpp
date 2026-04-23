#include "uart_connection.hpp"

using namespace nanoipc;

nanoipc::UartConnection::UartConnection(
	const std::string& device_path,
	const unsigned int baudrate,
	const SerialParity parity,
	const SerialStopBits stop_bits,
	const SerialDataBits data_bits,
	OnCharReceivedCallback on_char_received
):  m_device_path(device_path),
    m_baudrate(baudrate),
    m_parity(parity),
    m_stop_bits(stop_bits),
    m_data_bits(data_bits),
    m_on_char_received(on_char_received),
    m_is_open(false),
    m_listening(false) {
	if (device_path.empty()) {
		throw std::invalid_argument("device_path cannot be empty");
	}
	if (!on_char_received) {
		throw std::invalid_argument("on_char_received callback cannot be null");
	}
	if (baudrate == 0) {
		throw std::invalid_argument("baudrate must be non-zero");
	}
}

nanoipc::UartConnection::~UartConnection() noexcept {
	try {
		if (is_open()) {
			close();
		}
	} catch (...) {
		// Suppress exceptions in destructor
	}
}

void nanoipc::UartConnection::open() {
	if (is_open()) {
		throw std::logic_error("UartConnection is already open");
	}

	// Open the serial port
	if (m_serial_port.openDevice(m_device_path.c_str(), m_baudrate, m_data_bits, m_parity, m_stop_bits) != 1) {
		throw std::runtime_error("Failed to open UART device: " + m_device_path);
	}

	m_is_open.store(true);
	m_listening.store(true);

	// Start listening thread
	try {
		m_listen_thread = std::make_unique<std::thread>(&UartConnection::listen_thread_routine, this);
	} catch (...) {
		m_listening.store(false);
		m_is_open.store(false);
		m_serial_port.closeDevice();
		throw;
	}
}

void nanoipc::UartConnection::close() {
	if (!is_open()) {
		throw std::logic_error("UartConnection is not open");
	}

	// Signal listening thread to stop
	m_listening.store(false);

	// Wait for listening thread to finish
	if (m_listen_thread && m_listen_thread->joinable()) {
		m_listen_thread->join();
		m_listen_thread.reset();
	}

	// Close the serial port
    m_serial_port.closeDevice();
	m_is_open.store(false);
}

bool nanoipc::UartConnection::is_open() const {
	return m_is_open.load();
}

void nanoipc::UartConnection::write(const std::uint8_t *data, std::size_t data_size) {
	if (!is_open()) {
		throw std::runtime_error("UartConnection is not open");
	}
	if (!data) {
		throw std::invalid_argument("data pointer cannot be null");
	}
    // Write each byte
    if (m_serial_port.writeBytes(data, data_size) != 1) {
        throw std::runtime_error("Failed to write data to UART device");
    }
}

void nanoipc::UartConnection::listen_thread_routine() {
	char byte_buffer = 0;
	while (m_listening.load()) {
		// Read one character with timeout (100ms)
		// readChar returns 1 on success, 0 on timeout, -1 on error
		int result = m_serial_port.readChar(&byte_buffer, 100);
		if (result == 1) {
			m_on_char_received(static_cast<uint8_t>(byte_buffer));
		}
	}
}
