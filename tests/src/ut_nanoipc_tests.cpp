#include <climits>
#include <stdexcept>
#include <string>

#include "gtest/gtest.h"

#include "cobs.h"

#include "nanoipc_server.hpp"

using namespace nanoipc;

using ApiRequest = std::string;
using ApiResponse = int;
using TestNanoIpc = NanoIpcServer<ApiRequest, ApiResponse>;

static ApiRequest parse_request(const std::uint8_t *raw_data, const std::size_t raw_data_size);
static std::size_t serialize_response(const ApiResponse& response, std::uint8_t *dest_buff, const std::size_t dest_buff_capacity);

class VectorBuffer : public ReadBuffer {
public:
	VectorBuffer() = default;
	std::uint8_t pop_front() override {
		if (m_data.empty()) {
			throw std::runtime_error("pop_front called on empty buffer");
		}
		const auto front = m_data.front();
		m_data.erase(m_data.begin());
		return front;
	}
	std::size_t size() const override {
		return m_data.size();
	}
	std::uint8_t get(const std::size_t index) const override {
		if (index >= m_data.size()) {
			throw std::out_of_range("index out of range in get");
		}
		return m_data[index];
	}
	void push_back(const std::uint8_t byte) {
		m_data.push_back(byte);
	}
private:
	std::vector<std::uint8_t> m_data;
};

static void write_request_to_buffer(VectorBuffer *buffer, const ApiRequest& request);

TEST(ut_nanoipc_server, sanity) {
	// GIVEN
	VectorBuffer read_buffer;
	auto request_handler = [](const ApiRequest& request) -> ApiResponse {
		return static_cast<ApiResponse>(request.size());
	};
	auto serial_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
		// do nothing
	};
	// WHEN
	TestNanoIpc nano_ipc_server(request_handler, parse_request, serialize_response, serial_data_writer, &read_buffer);

	// THEN
	ASSERT_NO_THROW(nano_ipc_server.process());
}


ApiRequest parse_request(const std::uint8_t *raw_data, const std::size_t raw_data_size) {
	return ApiRequest(reinterpret_cast<const char*>(raw_data), raw_data_size);
}

std::size_t serialize_response(const ApiResponse& response, std::uint8_t *dest_buff, const std::size_t dest_buff_capacity) {
	if (dest_buff_capacity < sizeof(ApiResponse)) {
		throw std::runtime_error("insufficient buffer capacity in serialize_response");
	}
	for (auto i = 0; i < sizeof(ApiResponse); ++i) {
		dest_buff[i] = (response >> (i * CHAR_BIT)) & 0xFF;
	}
	return sizeof(ApiResponse);
}

void write_request_to_buffer(VectorBuffer *buffer, const ApiRequest& request) {
	enum { BUFFER_CAPACITY = 256 };
	if (request.size() >= BUFFER_CAPACITY) {
		throw std::invalid_argument("request too large to encode in buffer");
	}
	std::uint8_t temp_buffer[BUFFER_CAPACITY];
	std::size_t encoded_size = 0;
	for (auto i = std::size_t(0); i < request.size(); ++i) {
		temp_buffer[i] = static_cast<std::uint8_t>(request[i]);
	}
	
}