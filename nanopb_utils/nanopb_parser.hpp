#ifndef NANOPB_PARSER_HPP
#define NANOPB_PARSER_HPP

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "pb.h"
#include "pb_decode.h"

namespace nanoipc {
	/// @brief Parser that decodes nanopb messages into user `Message` objects.
	///
	/// `NanoPbParser` wraps nanopb decode functionality and a transformer that
	/// converts the generated `PbMessage` into the user-facing `Message` type.
	///
	/// Template parameters:
	/// - `Message`: the resulting user type returned by the parser
	/// - `PbMessage`: the nanopb-generated struct type used for decoding
	template <typename Message, typename PbMessage>
	class NanoPbParser {
	public:
		/// @brief Function type that transforms a decoded `PbMessage` into `Message`.
		using PbMessageToMessageTransformer = std::function<Message(const PbMessage&)>;
		/// @brief Optional initializer called with a zero-initialized `PbMessage` before decoding.
		///
		/// Use this to set up callback fields (for example to provide buffers for
		/// nanopb callback-encoded strings) prior to calling `pb_decode`.
		using PbMessageInitializer = std::function<void(PbMessage&)>;

		/// @brief Construct a `NanoPbParser`.
		///
		/// @param message_transformer Converts a decoded `PbMessage` into `Message`.
		/// @param nanopb_message_fields Pointer to the nanopb message field descriptor.
		/// @param pb_message_initializer Optional initializer invoked on a zeroed `PbMessage` before decoding.
		/// @throws std::invalid_argument if `message_transformer` or `nanopb_message_fields` are null/empty.
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

		/// @brief Decode `data` (raw protobuf bytes) into a `Message`.
		///
		/// This operator constructs a `PbMessage`, optionally initializes it via
		/// `m_pb_message_initializer`, then uses nanopb's `pb_decode` to populate
		/// the `PbMessage`. On success the `m_message_transformer` is applied and
		/// the resulting `Message` is returned. On failure a `std::runtime_error`
		/// is thrown containing the nanopb error string.
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
