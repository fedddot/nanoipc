#include "uart_connection.hpp"

namespace nanoipc {

UartConnection::UartConnection(
	const std::string& device_path,
	const unsigned int baudrate,
	const SerialParity parity,
	const SerialStopBits stop_bits,
	const SerialDataBits data_bits,
	OnCharReceivedCallback on_char_received
)
	: m_device_path(device_path),
	  m_baudrate(baudrate),
	  m_parity(parity),
	  m_stop_bits(stop_bits),
	  m_data_bits(data_bits),
	  m_on_char_received(on_char_received),
	  m_is_open(false),
	  m_listening(false),
	  m_serial_port(nullptr)
{
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

UartConnection::~UartConnection()
{
	try {
		if (is_open()) {
			close();
		}
	} catch (...) {
		// Suppress exceptions in destructor
	}
}

void UartConnection::open()
{
	if (is_open()) {
		throw std::logic_error("UartConnection is already open");
	}

	// Create serialib instance
	m_serial_port = std::make_unique<serialib>();

	// Open the serial port
	if (m_serial_port->openDevice(
		m_device_path.c_str(),
		m_baudrate,
		m_data_bits,
		m_parity,
		m_stop_bits) != 1)
	{
		m_serial_port.reset();
		throw std::runtime_error("Failed to open UART device: " + m_device_path);
	}

	m_is_open.store(true);
	m_listening.store(true);

	// Start listening thread
	try {
		m_listen_thread = std::make_unique<std::thread>(
			&UartConnection::listen_thread_routine,
			this
		);
	} catch (const std::exception& e) {
		m_listening.store(false);
		m_is_open.store(false);
		m_serial_port->closeDevice();
		m_serial_port.reset();
		throw std::runtime_error(std::string("Failed to start listening thread: ") + e.what());
	}
}

void UartConnection::close()
{
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
	if (m_serial_port) {
		m_serial_port->closeDevice();
		m_serial_port.reset();
	}

	m_is_open.store(false);
}

bool UartConnection::is_open() const
{
	return m_is_open.load();
}

void UartConnection::write(const std::uint8_t *data, std::size_t data_size)
{
	if (!is_open()) {
		throw std::runtime_error("UartConnection is not open");
	}

	if (!data && data_size > 0) {
		throw std::invalid_argument("data pointer cannot be null when data_size > 0");
	}

	if (data_size == 0) {
		return;  // Nothing to write
	}

	if (!m_serial_port) {
		throw std::runtime_error("Serial port handle is invalid");
	}

	// Write each byte
	for (std::size_t i = 0; i < data_size; ++i) {
		if (m_serial_port->writeChar(data[i]) == -1) {
			throw std::runtime_error("Failed to write data to UART device");
		}
	}
}

void UartConnection::listen_thread_routine()
{
	if (!m_serial_port) {
		return;
	}

	char byte_buffer = 0;
	while (m_listening.load()) {
		// Read one character with timeout (100ms)
		// readChar returns 1 on success, 0 on timeout, -1 on error
		int result = m_serial_port->readChar(&byte_buffer, 100);
		if (result == 1) {
			m_on_char_received(static_cast<uint8_t>(byte_buffer));
		}
	}
}

}  // namespace nanoipc
