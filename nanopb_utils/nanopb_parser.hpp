#ifndef NANOPB_PARSER_HPP
#define NANOPB_PARSER_HPP

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "pb.h"
#include "pb_decode.h"

namespace nanoipc {
	template <typename Message, typename PbMessage, std::size_t MAX_MSG_SIZE = 0xFF>
	class NanoPbParser {
	public:
		using PbMessageToMessageTransformer = std::function<Message(const PbMessage&)>;
		// Called with a zero-initialized PbMessage before pb_decode runs —
		// use this to install decode callbacks for callback-type fields (e.g. strings).
		using PbMessageInitializer = std::function<void(PbMessage&)>;

		NanoPbParser(
			const PbMessageToMessageTransformer& message_transformer,
			const pb_msgdesc_t* nanopb_message_fields,
			const PbMessageInitializer& pb_message_initializer = nullptr
		): m_message_transformer(message_transformer),
		   m_nanopb_message_fields(nanopb_message_fields),
		   m_pb_message_initializer(pb_message_initializer) {
			if (!m_message_transformer || !m_nanopb_message_fields) {
				throw std::invalid_argument("invalid arguments in NanoPbParser ctor");
			}
		}
		NanoPbParser(const NanoPbParser&) = default;
		NanoPbParser& operator=(const NanoPbParser&) = default;
		virtual ~NanoPbParser() noexcept = default;

		Message operator()(const std::uint8_t* data, std::size_t size) const {
			PbMessage pb_message = {};
			if (m_pb_message_initializer) {
				m_pb_message_initializer(pb_message);
			}
			pb_istream_t istream = pb_istream_from_buffer(data, size);
			if (!pb_decode(&istream, m_nanopb_message_fields, &pb_message)) {
				throw std::runtime_error(
					"failed to decode protocol buffer: " + std::string(PB_GET_ERROR(&istream))
				);
			}
			return m_message_transformer(pb_message);
		}

		Message operator()(const std::vector<std::uint8_t>& data) const {
			return (*this)(data.data(), data.size());
		}

	private:
		PbMessageToMessageTransformer m_message_transformer;
		const pb_msgdesc_t* m_nanopb_message_fields;
		PbMessageInitializer m_pb_message_initializer;
	};
}

#endif // NANOPB_PARSER_HPP
