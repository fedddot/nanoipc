#ifndef	NANOPB_SERIALIZER_HPP
#define	NANOPB_SERIALIZER_HPP

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

namespace nanoipc {
	template <typename Message, typename PbMessage>
	class NanoPbSerializer {
	public:
		using MessageToPbTransformer = std::function<PbMessage(const Message& message)>;

		NanoPbSerializer(
			const MessageToPbTransformer& message_transformer
		): m_message_transformer(message_transformer) {
			if (!m_message_transformer) {
				throw std::invalid_argument("invalid arguments in NanoPbSerializer ctor");
			}
		}
		NanoPbSerializer(const NanoPbSerializer&) = default;
		NanoPbSerializer& operator=(const NanoPbSerializer&) = default;
		virtual ~NanoPbSerializer() noexcept = default;

		std::vector<std::uint8_t> operator()(const Message& message) const {
			PbMessage pb_message = m_message_transformer(message);
			throw std::runtime_error("NanoPbSerializer::operator() not implemented");
		}
	private:
		MessageToPbTransformer m_message_transformer;
	};
}

#endif // NANOPB_SERIALIZER_HPP
