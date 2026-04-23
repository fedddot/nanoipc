#ifndef	PB_MESSAGE_WRITER_HPP
#define	PB_MESSAGE_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "writer.hpp"

namespace nanoipc {
    template <typename Tpb_msg>
	class PbMessageWriter: public Writer<Tpb_msg> {
	public:
		PbMessageWriter(const Writer<std::vector<std::uint8_t>> *frame_writer): m_frame_writer(frame_writer) {
			if (!m_frame_writer) {
				throw std::invalid_argument("invalid arguments in PbMessageWriter ctor");
			}
		}
		PbMessageWriter(const PbMessageWriter&) = default;
		PbMessageWriter& operator=(const PbMessageWriter&) = default;

		void write(const Tpb_msg& data) const override {

		}

	private:
        const Writer<std::vector<std::uint8_t>> *m_frame_writer;
	};
}

#endif // PB_MESSAGE_WRITER_HPP
