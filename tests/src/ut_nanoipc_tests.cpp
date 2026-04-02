#include <climits>
#include <stdexcept>
#include <string>

#include "gtest/gtest.h"
#include <vector>

#include "nanoipc_client.hpp"
#include "nanoipc_server.hpp"
#include "nanoipc_utils.hpp"

using namespace nanoipc;

using ApiRequest = std::string;
using ApiResponse = int;
using TestNanoIpc = NanoIpcServer<ApiRequest, ApiResponse>;

static ApiRequest parse_request(const std::uint8_t *raw_data, const std::size_t raw_data_size);
static std::vector<std::uint8_t> serialize_response(const ApiResponse& response);

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

TEST(ut_nanoipc_server, sanity) {
	// GIVEN
	const auto test_request = ApiRequest("hello world");
	auto request_handler = [](const ApiRequest& request) -> ApiResponse {
		return static_cast<ApiResponse>(request.size());
	};
	auto serial_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
		// do nothing
	};

	// WHEN
	VectorBuffer read_buffer;
	const auto encoded_request = encode_frame((std::uint8_t *)(test_request.c_str()), test_request.size());
	for (const auto ch: encoded_request) {
		read_buffer.push_back(ch);
	}
	TestNanoIpc nano_ipc_server(request_handler, parse_request, serialize_response, serial_data_writer, &read_buffer);

	// THEN
	ASSERT_NO_THROW(nano_ipc_server.process());
}


ApiRequest parse_request(const std::uint8_t *raw_data, const std::size_t raw_data_size) {
	return ApiRequest(reinterpret_cast<const char*>(raw_data), raw_data_size);
}

std::vector<std::uint8_t> serialize_response(const ApiResponse& response) {
	std::vector<std::uint8_t> serial_response(sizeof(ApiResponse));
	for (auto i = 0; i < sizeof(ApiResponse); ++i) {
		serial_response[i] = (response >> (i * CHAR_BIT)) & 0xFF;
	}
	return serial_response;
}

static std::vector<std::uint8_t> serialize_request(const ApiRequest& request) {
	return std::vector<std::uint8_t>(request.begin(), request.end());
}

static ApiResponse parse_response(const std::uint8_t *raw_data, const std::size_t raw_data_size) {
	ApiResponse response = 0;
	for (auto i = std::size_t(0); i < raw_data_size && i < sizeof(ApiResponse); ++i) {
		response |= static_cast<ApiResponse>(raw_data[i]) << (i * CHAR_BIT);
	}
	return response;
}

TEST(ut_nanoipc_client, sanity) {
	using TestNanoIpcClient = NanoIpcClient<ApiRequest, ApiResponse>;

	// GIVEN
	const auto test_request = ApiRequest("hello world");
	const auto expected_response = static_cast<ApiResponse>(test_request.size());

	VectorBuffer response_buffer;
	const auto encoded_response = encode_frame(serialize_response(expected_response).data(), sizeof(ApiResponse));
	for (const auto byte : encoded_response) {
		response_buffer.push_back(byte);
	}

	VectorBuffer request_sink;
	auto serial_data_writer = [&request_sink](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
		for (auto i = std::size_t(0); i < raw_data_size; ++i) {
			request_sink.push_back(raw_data[i]);
		}
	};

	// WHEN
	TestNanoIpcClient client(serialize_request, parse_response, serial_data_writer, &response_buffer);
	const auto response = client.send(test_request);

	// THEN
	ASSERT_EQ(response, expected_response);
}