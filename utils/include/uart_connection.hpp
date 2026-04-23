#ifndef UART_CONNECTION_HPP
#define UART_CONNECTION_HPP

#include <cstdint>
#include <cstddef>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include "serialib.h"

namespace nanoipc {
	/// @brief Manages a Linux UART connection using the serialib library.
	///
	/// UartConnection provides an abstraction for communicating with UART devices
	/// on Linux systems. It uses the serialib library for low-level UART access
	/// and implements a background thread for listening to incoming data.
	///
	/// Usage example:
	/// @code
	/// auto on_char_received = [](uint8_t c) {
	///     std::cout << "Received: " << c << std::endl;
	/// };
	///
	/// UartConnection uart("/dev/ttyUSB0", 115200, SERIAL_PARITY_NONE, 
	///                     SERIAL_STOPBITS_1, SERIAL_DATABITS_8, on_char_received);
	/// uart.open();
	/// uart.write(reinterpret_cast<const uint8_t*>("Hello"), 5);
	/// uart.close();
	/// @endcode
	///
	/// Notes:
	/// - The callback function is invoked from the internal reading thread. 
	///   If the callback performs long-running operations or accesses shared 
	///   state, appropriate synchronization is required.
	/// - The open() method starts a background thread for listening to incoming data.
	/// - The close() method stops the listening thread and closes the UART device.
	class UartConnection {
	public:
		/// @brief Callback function type for handling received characters.
		/// @param character The received character.
		using OnCharReceivedCallback = std::function<void(const std::uint8_t)>;

		/// @brief Construct a UartConnection.
		///
		/// @param device_path Path to the UART device (e.g., "/dev/ttyUSB0", "/dev/ttyS0").
		/// @param baudrate Baud rate for communication (e.g., 9600, 115200).
		/// @param parity Parity setting (SERIAL_PARITY_NONE, SERIAL_PARITY_EVEN, or SERIAL_PARITY_ODD).
		/// @param stop_bits Number of stop bits (SERIAL_STOPBITS_1 or SERIAL_STOPBITS_2).
		/// @param data_bits Number of data bits (SERIAL_DATABITS_5, 6, 7, or 8).
		/// @param on_char_received Callback function to invoke when a character is received.
		///                         Must not be null.
		/// @throws std::invalid_argument if device_path is empty or on_char_received is null.
		UartConnection(
			const std::string& device_path,
			const unsigned int baudrate,
			const SerialParity parity,
			const SerialStopBits stop_bits,
			const SerialDataBits data_bits,
			OnCharReceivedCallback on_char_received
		);

		/// @brief Destructor. Ensures resources are properly released.
		~UartConnection();

		UartConnection(const UartConnection&) = delete;
		UartConnection& operator=(const UartConnection&) = delete;
		UartConnection(UartConnection&& other) = delete;
		UartConnection& operator=(UartConnection&& other) = delete;

		/// @brief Open the UART connection and start listening for incoming data.
		///
		/// This method initializes the UART device with the configured parameters
		/// and starts a background thread that continuously reads data from the device.
		/// The callback function is invoked for each received character.
		///
		/// @throws std::runtime_error if the device cannot be opened or if the listening thread cannot be started.
		/// @throws std::logic_error if the connection is already open.
		void open();

		/// @brief Close the UART connection and stop listening for incoming data.
		///
		/// This method stops the listening thread and closes the UART device.
		/// After this call, no more callbacks will be invoked.
		///
		/// @throws std::runtime_error if the device cannot be closed.
		/// @throws std::logic_error if the connection is not open.
		void close();

		/// @brief Check if the UART connection is currently open.
		///
		/// @return true if the connection is open, false otherwise.
		bool is_open() const;

		/// @brief Write data to the UART connection.
		///
		/// @param data Pointer to the data to write. Must not be null.
		/// @param data_size Number of bytes to write.
		/// @throws std::runtime_error if the write operation fails or if the connection is not open.
		/// @throws std::invalid_argument if data is null.
		void write(const std::uint8_t *data, std::size_t data_size);

	private:
		std::string m_device_path;
		unsigned int m_baudrate;
		SerialParity m_parity;
		SerialStopBits m_stop_bits;
		SerialDataBits m_data_bits;
		OnCharReceivedCallback m_on_char_received;
		std::atomic<bool> m_is_open;
		std::atomic<bool> m_listening;
		std::unique_ptr<std::thread> m_listen_thread;
		serialib m_serial_port;

		/// @brief Internal method that runs in the listening thread.
		void listen_thread_routine();
	};
}

#endif // UART_CONNECTION_HPP

