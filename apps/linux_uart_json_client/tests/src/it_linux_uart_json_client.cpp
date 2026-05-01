#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <sys/wait.h>

#include <gtest/gtest.h>

#include "virtual_uart.hpp"
#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "json_message_reader.hpp"
#include "json_message_writer.hpp"
#include "ring_buffer.hpp"
#include "json/value.h"

#ifndef APP_BINARY
#  error "APP_BINARY must be defined to the path of the app under test"
#endif

using namespace nanoipc;
using namespace nanoipc_testing;

static void write_json_to_file(const std::string& filepath, const Json::Value& json) {
    std::ofstream f(filepath);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot create JSON file: " + filepath);
    }
    Json::StreamWriterBuilder builder;
    f << Json::writeString(builder, json);
}

static Json::Value read_json_from_file(const std::string& filepath) {
    std::ifstream f(filepath);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open JSON file: " + filepath);
    }
    Json::Value json;
    Json::CharReaderBuilder builder;
    std::string errors;
    if (!Json::parseFromStream(builder, f, &json, &errors)) {
        throw std::runtime_error("Failed to parse JSON: " + errors);
    }
    return json;
}

TEST_F(VirtualUart, SendsJsonRequestAndReceivesJsonResponse) {
    // Create JSON request and response payloads
    Json::Value request_json;
    request_json["id"] = 42;
    request_json["command"] = "test_command";
    request_json["data"]["value"] = 3.14159;
    request_json["data"]["name"] = "test_data";

    Json::Value response_json;
    response_json["status"] = "success";
    response_json["result"] = 100;
    response_json["message"] = "Operation completed";

    // Use PID-unique temp file paths to avoid collisions in parallel runs.
    const std::string req_file = "/tmp/it_json_req_" + std::to_string(::getpid()) + ".json";
    const std::string rsp_file = "/tmp/it_json_rsp_" + std::to_string(::getpid()) + ".json";

    // Write request JSON to file
    write_json_to_file(req_file, request_json);

    const std::string cmd = std::string(APP_BINARY)
        + " --port "     + slave_path()
        + " --baud 9600"
        + " --request "  + req_file
        + " --response " + rsp_file
        + " --timeout 10";

    int system_ret = -1;
    std::thread app_thread([&]() { system_ret = std::system(cmd.c_str()); });

    // --- act as the device on the master side ---

    // Read the COBS frame the app sent (blocks with timeout).
    std::vector<std::uint8_t> raw_frame;
    bool read_ok = false;
    try {
        raw_frame = read_until_byte(0x00, 5000);
        read_ok = true;
    } catch (...) {}

    if (!read_ok) {
        app_thread.join();
        ::unlink(req_file.c_str());
        FAIL() << "Timeout waiting for request from app";
    }

    // Decode the received COBS frame
    RingBuffer<256> decode_buf;
    for (auto b : raw_frame) { decode_buf.push_back(b); }
    CobsFrameReader decoder(&decode_buf);
    const auto decoded_request = decoder.read();
    ASSERT_TRUE(decoded_request.has_value()) << "Failed to decode COBS frame from app";

    // Parse decoded frame as JSON
    Json::Value received_request;
    Json::CharReaderBuilder reader_builder;
    std::string parse_errors;
    const auto decoded_bytes = decoded_request.value();
    std::string json_str(decoded_bytes.begin(), decoded_bytes.end());
    std::istringstream json_stream(json_str);
    ASSERT_TRUE(Json::parseFromStream(reader_builder, json_stream, &received_request, &parse_errors))
        << "Failed to parse received JSON: " << parse_errors;

    // Verify received request matches sent request
    ASSERT_EQ(received_request["id"].asInt(), request_json["id"].asInt());
    ASSERT_EQ(received_request["command"].asString(), request_json["command"].asString());
    ASSERT_FLOAT_EQ(received_request["data"]["value"].asFloat(), request_json["data"]["value"].asFloat());
    ASSERT_EQ(received_request["data"]["name"].asString(), request_json["data"]["name"].asString());

    // Encode the response JSON and send it to the app.
    Json::StreamWriterBuilder json_writer_builder;
    const std::string response_str = Json::writeString(json_writer_builder, response_json);
    const std::vector<std::uint8_t> response_bytes(response_str.begin(), response_str.end());

    std::vector<std::uint8_t> encoded_response;
    CobsFrameWriter encoder([&encoded_response](const std::uint8_t* data, std::size_t len) {
        encoded_response.insert(encoded_response.end(), data, data + len);
    });
    encoder.write(response_bytes);
    write_data(encoded_response);

    // Wait for the app to exit and assert success.
    app_thread.join();
    ASSERT_EQ(WEXITSTATUS(system_ret), 0) << "App exited with non-zero status";

    // Verify the response file content.
    const auto received_response = read_json_from_file(rsp_file);
    ASSERT_EQ(received_response["status"].asString(), response_json["status"].asString());
    ASSERT_EQ(received_response["result"].asInt(), response_json["result"].asInt());
    ASSERT_EQ(received_response["message"].asString(), response_json["message"].asString());

    ::unlink(req_file.c_str());
    ::unlink(rsp_file.c_str());
}
