#ifndef	NANOPB_SERIALIZER_HPP
#define	NANOPB_SERIALIZER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "nanoipc_utils.hpp"

namespace nanoipc {
	template <typename Message>
	class NanoPbSerializer {
	public:
		using MessageSerializer = std::function<std::vector<std::uint8_t>(const Message& message)>;
		using RawDataSerializer = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoPbSerializer(
			const MessageSerializer& message_serializer,
			const RawDataSerializer& raw_data_writer
		): m_message_serializer(message_serializer), m_raw_data_writer(raw_data_writer) {
			if (!m_message_serializer || !m_raw_data_writer) {
				throw std::invalid_argument("invalid arguments in NanoPbSerializer ctor");
			}
		}
		NanoPbSerializer(const NanoPbSerializer&) = default;
		NanoPbSerializer& operator=(const NanoPbSerializer&) = default;
		virtual ~NanoPbSerializer() noexcept = default;

		void write(const Message& message) {
			const auto serial_message = m_message_serializer(message);
			const auto encoded_message = encode_frame(serial_message.data(), serial_message.size());
			m_raw_data_writer(encoded_message.data(), encoded_message.size());
		}

	private:
		MessageSerializer m_message_serializer;
		RawDataSerializer m_raw_data_writer;
	};
}

#endif // NANOPB_SERIALIZER_HPP
