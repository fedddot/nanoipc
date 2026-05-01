#include <chrono>
#include <cstdint>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>

#include "virtual_uart.hpp"
#include "linux_uart_cobs_frame_reader.hpp"
#include "linux_uart_cobs_frame_writer.hpp"
#include "cobs_frame_reader.hpp"
#include "ring_buffer.hpp"
#include "serialib.h"

using namespace nanoipc;
using namespace nanoipc_testing;

// COBS encoding of {0x01}: [0x02, 0x01, 0x00]
// code=0x02 means 2 octets until next delimiter (i.e. one data byte), then 0x01, then 0x00 delimiter.
TEST_F(VirtualUart, WriterSendsCobsEncodedFrame) {
    serialib uart;
    ASSERT_EQ(uart.openDevice(slave_path().c_str(), 9600), 1);

    LinuxUartCobsFrameWriter writer(&uart);
    const std::vector<std::uint8_t> payload{0x01};
    ASSERT_NO_THROW(writer.write(payload));

    const auto raw = read_cobs_frame_bytes();

    // Verify exact COBS encoding of {0x01}: [0x02, 0x01, 0x00]
    const std::vector<std::uint8_t> expected_raw{0x02, 0x01, 0x00};
    EXPECT_EQ(raw, expected_raw);

    // Also verify that decoding recovers the original payload
    RingBuffer<64> buf;
    for (auto b : raw) { buf.push_back(b); }
    CobsFrameReader decoder(&buf);
    const auto decoded = decoder.read();
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded.value(), payload);

    uart.closeDevice();
}

TEST_F(VirtualUart, ReaderReceivesCobsFrame) {
    serialib uart;
    ASSERT_EQ(uart.openDevice(slave_path().c_str(), 9600), 1);

    LinuxUartCobsFrameReader<256> reader(&uart);
    const std::vector<std::uint8_t> payload{0x01, 0x02, 0x03};

    // Encode using a known-good sequence: inject {0x04, 0x01, 0x02, 0x03, 0x00}
    // which is the canonical COBS encoding of {0x01, 0x02, 0x03}.
    const std::vector<std::uint8_t> encoded_frame{0x04, 0x01, 0x02, 0x03, 0x00};
    write_data(encoded_frame);

    // PTY byte propagation is not instantaneous: poll until the reader
    // delivers the frame or a 2-second deadline expires.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    std::optional<std::vector<std::uint8_t>> result;
    while (!result.has_value() && std::chrono::steady_clock::now() < deadline) {
        result = reader.read();
    }
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), payload);

    uart.closeDevice();
}

TEST_F(VirtualUart, ReaderReturnsNulloptWhenNoDataAvailable) {
    serialib uart;
    ASSERT_EQ(uart.openDevice(slave_path().c_str(), 9600), 1);

    LinuxUartCobsFrameReader<256> reader(&uart);

    const auto result = reader.read();
    EXPECT_FALSE(result.has_value());

    uart.closeDevice();
}

TEST_F(VirtualUart, ReaderHandlesFragmentedFrame) {
    serialib uart;
    ASSERT_EQ(uart.openDevice(slave_path().c_str(), 9600), 1);

    LinuxUartCobsFrameReader<256> reader(&uart);
    const std::vector<std::uint8_t> payload{0x01, 0x02, 0x03};

    // Send the first part of the COBS frame (no delimiter yet)
    write_data({0x04, 0x01, 0x02});
    // Wait for PTY propagation so the partial bytes are actually read into
    // the ring buffer; then verify that no complete frame is returned yet.
    ::usleep(50'000);
    const auto partial = reader.read();
    EXPECT_FALSE(partial.has_value());

    // Send the remainder including the COBS delimiter
    write_data({0x03, 0x00});
    // Poll until the reader delivers the full frame or the deadline expires.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    std::optional<std::vector<std::uint8_t>> result;
    while (!result.has_value() && std::chrono::steady_clock::now() < deadline) {
        result = reader.read();
    }
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), payload);

    uart.closeDevice();
}
