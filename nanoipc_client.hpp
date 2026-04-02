#ifndef	NANOIPC_CLIENT_HPP
#define	NANOIPC_CLIENT_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "nanoipc_read_buffer.hpp"
#include "nanoipc_utils.hpp"

namespace nanoipc {
	template <typename IpcRequest, typename IpcResponse>
	class NanoIpcClient {
	public:
		using RequestSerializer = std::function<std::vector<std::uint8_t>(const IpcRequest& request)>;
		using ResponseParser = std::function<IpcResponse(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;
		using SerialDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoIpcClient(
			const RequestSerializer& request_serializer,
			const ResponseParser& response_parser,
			const SerialDataWriter& serial_data_writer,
			ReadBuffer *read_buffer
		): m_request_serializer(request_serializer), m_response_parser(response_parser), m_serial_data_writer(serial_data_writer), m_read_buffer(read_buffer) {
			if (!m_request_serializer || !m_response_parser || !m_serial_data_writer || !m_read_buffer) {
				throw std::invalid_argument("invalid arguments in NanoIpcClient ctor");
			}
		}
		NanoIpcClient(const NanoIpcClient&) = default;
		NanoIpcClient& operator=(const NanoIpcClient&) = default;
		virtual ~NanoIpcClient() noexcept = default;

		IpcResponse send(const IpcRequest& request) {
			const auto serial_request = m_request_serializer(request);
			const auto encoded_request = encode_frame(serial_request.data(), serial_request.size());
			m_serial_data_writer(encoded_request.data(), encoded_request.size());

			const auto encoded_frame = read_encoded_frame(m_read_buffer);
			if (!encoded_frame.has_value()) {
				throw std::runtime_error("no response received");
			}
			const auto decoded_frame = decode_frame(encoded_frame->data(), encoded_frame->size());
			return m_response_parser(decoded_frame.data(), decoded_frame.size());
		}

	private:
		RequestSerializer m_request_serializer;
		ResponseParser m_response_parser;
		SerialDataWriter m_serial_data_writer;
		ReadBuffer *m_read_buffer;
	};
}

#endif // NANOIPC_CLIENT_HPP
