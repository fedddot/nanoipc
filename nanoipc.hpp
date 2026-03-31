#ifndef	NANOIPC_HPP
#define	NANOIPC_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace nanoipc {
	template <typename IpcRequest, typename IpcResponse, std::size_t BUFF_CAPACITY>
	class NanoIpcServer {
	public:
		using RequestHandler = std::function<IpcResponse(const IpcRequest&)>;
		using RequestParser = std::function<IpcRequest(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;
		using ResponseSerializer = std::function<std::size_t(std::uint8_t *buff, const std::size_t buff_capacity)>;
		using SerialDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;
		NanoIpcServer(
			const RequestHandler& request_handler,
			const RequestParser& request_parser,
			const ResponseSerializer& response_serializer,
			const SerialDataWriter& serial_data_writer
		);
		NanoIpcServer(const NanoIpcServer&) = delete;
		NanoIpcServer& operator=(const NanoIpcServer&) = delete;

		virtual ~NanoIpcServer() noexcept = default;
		void feed(const std::uint8_t ch);
	private:
		std::array<std::uint8_t, BUFF_CAPACITY> m_buffer;
	};
}

#endif // NANOIPC_HPP