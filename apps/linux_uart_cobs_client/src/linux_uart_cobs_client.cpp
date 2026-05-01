#include <chrono>
#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

#include <CLI/CLI.hpp>

#include "linux_uart_cobs_frame_reader.hpp"
#include "linux_uart_cobs_frame_writer.hpp"
#include "serialib.h"

using namespace nanoipc;

enum: std::size_t { BUFF_SIZE = 4096UL };

SerialParity parse_parity(const std::string& parity_str) {
    if (parity_str == "NONE") return SerialParity::SERIAL_PARITY_NONE;
    if (parity_str == "EVEN") return SerialParity::SERIAL_PARITY_EVEN;
    if (parity_str == "ODD") return SerialParity::SERIAL_PARITY_ODD;
    if (parity_str == "MARK") return SerialParity::SERIAL_PARITY_MARK;
    if (parity_str == "SPACE") return SerialParity::SERIAL_PARITY_SPACE;
    throw std::runtime_error("Invalid parity: " + parity_str);
}

SerialDataBits parse_data_bits(unsigned int bits) {
    switch (bits) {
        case 5: return SerialDataBits::SERIAL_DATABITS_5;
        case 6: return SerialDataBits::SERIAL_DATABITS_6;
        case 7: return SerialDataBits::SERIAL_DATABITS_7;
        case 8: return SerialDataBits::SERIAL_DATABITS_8;
        case 16: return SerialDataBits::SERIAL_DATABITS_16;
        default: throw std::runtime_error("Invalid data bits: " + std::to_string(bits));
    }
}

SerialStopBits parse_stop_bits(const std::string& stop_bits_str) {
    if (stop_bits_str == "1") return SerialStopBits::SERIAL_STOPBITS_1;
    if (stop_bits_str == "1.5") return SerialStopBits::SERIAL_STOPBITS_1_5;
    if (stop_bits_str == "2") return SerialStopBits::SERIAL_STOPBITS_2;
    throw std::runtime_error("Invalid stop bits: " + stop_bits_str);
}

std::vector<std::uint8_t> read_binary_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open request file: " + filepath);
    }
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

int main(int argc, char* argv[]) {
    CLI::App app("UART COBS Client - Send and receive raw binary payloads over UART using COBS framing");

    std::string port = "/dev/ttyACM0";
    unsigned int baud = 9600;
    std::string parity_str = "NONE";
    unsigned int data_bits = 8;
    std::string stop_bits_str = "1";
    std::string request_path;
    std::string response_path;
    unsigned int timeout_sec = 3;

    app.add_option("--port", port, "Device path (e.g., /dev/ttyACM0, /dev/ttyUSB0)")
        ->default_val("/dev/ttyACM0");

    app.add_option("--baud", baud, "Baud rate (e.g., 9600, 115200, 230400)")
        ->default_val("9600")
        ->check(CLI::PositiveNumber);

    app.add_option("--parity", parity_str, "Parity type: NONE, EVEN, ODD, MARK, SPACE")
        ->default_val("NONE")
        ->check(CLI::IsMember({"NONE", "EVEN", "ODD", "MARK", "SPACE"}));

    app.add_option("--data-bits", data_bits, "Number of data bits: 5, 6, 7, 8, 16")
        ->default_val("8")
        ->check(CLI::IsMember({5, 6, 7, 8, 16}));

    app.add_option("--stop-bits", stop_bits_str, "Number of stop bits: 1, 1.5, 2")
        ->default_val("1")
        ->check(CLI::IsMember({"1", "1.5", "2"}));

    app.add_option("--request", request_path, "Path to binary file containing the raw request payload")
        ->required();

    app.add_option("--response", response_path, "Path to write the raw binary response payload")
        ->required();

    app.add_option("--timeout", timeout_sec, "Timeout in seconds waiting for a response")
        ->default_val("3")
        ->check(CLI::NonNegativeNumber);

    CLI11_PARSE(app, argc, argv);

    SerialParity parity = parse_parity(parity_str);
    SerialDataBits db = parse_data_bits(data_bits);
    SerialStopBits sb = parse_stop_bits(stop_bits_str);

    std::vector<std::uint8_t> request_payload;
    try {
        request_payload = read_binary_file(request_path);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    std::cout << "Sending " << request_payload.size() << " byte(s) from " << request_path << std::endl;

    serialib uart;
    const auto open_result = uart.openDevice(port.c_str(), baud, db, parity, sb);
    if (open_result != 1) {
        std::cerr << "Failed to open serial port: " << port << std::endl;
        return 1;
    }

    LinuxUartCobsFrameReader<BUFF_SIZE> cobs_reader(&uart);
    LinuxUartCobsFrameWriter cobs_writer(&uart);

    uart.flushReceiver();
    cobs_writer.write(request_payload);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_sec);
    while (true) {
        if (std::chrono::steady_clock::now() >= deadline) {
            std::cerr << "Timeout: no response received within " << timeout_sec << " second(s)" << std::endl;
            uart.closeDevice();
            return 1;
        }
        const auto response = cobs_reader.read();
        if (!response.has_value()) {
            continue;
        }
        const auto& payload = response.value();
        std::cout << "Received " << payload.size() << " byte(s), writing to " << response_path << std::endl;

        std::ofstream response_file(response_path, std::ios::binary);
        if (!response_file.is_open()) {
            std::cerr << "Failed to open response file for writing: " << response_path << std::endl;
            uart.closeDevice();
            return 1;
        }
        response_file.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
        break;
    }

    uart.closeDevice();
    return 0;
}
