#ifndef	PB_MESSAGE_READER_HPP
#define	PB_MESSAGE_READER_HPP

#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

#include "pb.h"
#include "pb_decode.h"
#include "reader.hpp"

namespace nanoipc {
    template <typename Tpb_msg>
	class PbMessageReader: public Reader<Tpb_msg> {
	public:
        using PbMessageInitializer = std::function<void(Tpb_msg *)>;
		PbMessageReader(
            Reader<std::vector<std::uint8_t>> *frame_reader,
			const pb_msgdesc_t *nanopb_message_fields,
			const PbMessageInitializer& pb_message_initializer = nullptr
        ): m_frame_reader(frame_reader), m_nanopb_message_fields(nanopb_message_fields), m_pb_message_initializer(pb_message_initializer) {
			if (!m_frame_reader || !m_nanopb_message_fields) {
				throw std::invalid_argument("invalid arguments in PbMessageReader ctor");
			}
		}
		PbMessageReader(const PbMessageReader&) = default;
		PbMessageReader& operator=(const PbMessageReader&) = default;

		std::optional<Tpb_msg> read() override {
            const auto frame_data = m_frame_reader->read();
            if (!frame_data.has_value()) {
                return std::nullopt;
            }
            Tpb_msg pb_message;
			if (m_pb_message_initializer) {
				m_pb_message_initializer(&pb_message);
			}
			pb_istream_t istream = pb_istream_from_buffer(frame_data->data(), frame_data->size());
			if (!pb_decode(&istream, m_nanopb_message_fields, &pb_message)) {
				throw std::runtime_error(
					"failed to decode protocol buffer: " + std::string(PB_GET_ERROR(&istream))
				);
			}
            return std::make_optional(pb_message);
		}

	private:
        Reader<std::vector<std::uint8_t>> *m_frame_reader;
		const pb_msgdesc_t *m_nanopb_message_fields;
		PbMessageInitializer m_pb_message_initializer;
	};
}

#endif // PB_MESSAGE_READER_HPP