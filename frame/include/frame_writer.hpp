#ifndef	NANOIPC_WRITER_HPP
#define	NANOIPC_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "nanoipc_utils.hpp"

namespace nanoipc {
	/// @brief Generic writer that serializes `Message` instances and writes encoded frames.
	///
	/// `NanoIpcWriter` converts user-provided `Message` objects into raw bytes
	/// using a `MessageSerializer`, encodes those bytes into a transport-ready
	/// frame via `encode_frame`, and finally emits the encoded bytes using the
	/// provided `RawDataWriter` callback.
	template <typename Message>
	class NanoIpcWriter {
	public:
		/// @brief Function type that serializes a `Message` into raw bytes.
		using MessageSerializer = std::function<std::vector<std::uint8_t>(const Message& message)>;
		/// @brief Function type that writes raw bytes to the transport.
		using RawDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		/// @brief Construct a `NanoIpcWriter`.
		///
		/// @param message_serializer Callable that converts a `Message` into a vector of raw bytes.
		/// @param raw_data_writer Callable that emits raw bytes to the underlying transport.
		/// @throws std::invalid_argument if either callback is empty.
		NanoIpcWriter(
			const MessageSerializer& message_serializer,
			const RawDataWriter& raw_data_writer
		): m_message_serializer(message_serializer), m_raw_data_writer(raw_data_writer) {
			if (!m_message_serializer || !m_raw_data_writer) {
				throw std::invalid_argument("invalid arguments in NanoIpcWriter ctor");
			}
		}
		NanoIpcWriter(const NanoIpcWriter&) = default;
		NanoIpcWriter& operator=(const NanoIpcWriter&) = default;
		virtual ~NanoIpcWriter() noexcept = default;

		/// @brief Serialize and write `message` as an encoded frame.
		///
		/// The message is first serialized via the `MessageSerializer`, then
		/// encoded with `encode_frame` to produce a transport-ready frame, and
		/// finally emitted through the `RawDataWriter` callback.
		///
		/// @param message The message to serialize and write.
		void write(const Message& message) {
			const auto serial_message = m_message_serializer(message);
			const auto encoded_message = encode_frame(serial_message.data(), serial_message.size());
			m_raw_data_writer(encoded_message.data(), encoded_message.size());
		}

	private:
		MessageSerializer m_message_serializer;
		RawDataWriter m_raw_data_writer;
	};
}

#endif // NANOIPC_WRITER_HPP
