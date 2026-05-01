#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include <sys/wait.h>

#include <gtest/gtest.h>

#include "virtual_uart.hpp"
#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "ring_buffer.hpp"

#ifndef APP_BINARY
#  error "APP_BINARY must be defined to the path of the app under test"
#endif

using namespace nanoipc;
using namespace nanoipc_testing;

TEST_F(VirtualUart, SendsRequestAndReceivesResponse) {
    const std::vector<std::uint8_t> request_payload  = {0xDE, 0xAD, 0xBE, 0xEF};
    const std::vector<std::uint8_t> response_payload = {0xCA, 0xFE};

    // Use PID-unique temp file paths to avoid collisions in parallel runs.
    const std::string req_file = "/tmp/it_cobs_req_" + std::to_string(::getpid()) + ".bin";
    const std::string rsp_file = "/tmp/it_cobs_rsp_" + std::to_string(::getpid()) + ".bin";

    {
        std::ofstream f(req_file, std::ios::binary);
        ASSERT_TRUE(f.is_open()) << "Cannot create request file: " << req_file;
        f.write(reinterpret_cast<const char*>(request_payload.data()),
                static_cast<std::streamsize>(request_payload.size()));
    }

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

    // Decode the received COBS frame and verify it matches the request.
    RingBuffer<256> decode_buf;
    for (auto b : raw_frame) { decode_buf.push_back(b); }
    CobsFrameReader decoder(&decode_buf);
    const auto decoded_request = decoder.read();
    ASSERT_TRUE(decoded_request.has_value()) << "Failed to decode COBS frame from app";
    ASSERT_EQ(decoded_request.value(), request_payload);

    // Encode the response payload and send it to the app.
    std::vector<std::uint8_t> encoded_response;
    CobsFrameWriter encoder([&encoded_response](const std::uint8_t* data, std::size_t len) {
        encoded_response.insert(encoded_response.end(), data, data + len);
    });
    encoder.write(response_payload);
    write_data(encoded_response);

    // Wait for the app to exit and assert success.
    app_thread.join();
    ASSERT_EQ(WEXITSTATUS(system_ret), 0) << "App exited with non-zero status";

    // Verify the response file content.
    std::ifstream rsp_in(rsp_file, std::ios::binary);
    ASSERT_TRUE(rsp_in.is_open()) << "Cannot open response file: " << rsp_file;
    const std::vector<std::uint8_t> rsp_contents(
        (std::istreambuf_iterator<char>(rsp_in)),
        std::istreambuf_iterator<char>()
    );
    ASSERT_EQ(rsp_contents, response_payload);

    ::unlink(req_file.c_str());
    ::unlink(rsp_file.c_str());
}

