#include <cstddef>
#include <iostream>
#include <fstream>

#include <CLI/CLI.hpp>

#include "linux_uart_cobs_frame_reader.hpp"
#include "linux_uart_cobs_frame_writer.hpp"
#include "json_message_reader.hpp"
#include "json_message_writer.hpp"
#include "serialib.h"
#include "json/value.h"

using namespace nanoipc;

enum: std::size_t { BUFF_SIZE = 1024UL };

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

Json::Value read_json_from_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open request file: " + filepath);
    }
    
    Json::Value json;
    Json::CharReaderBuilder reader_builder;
    std::string errors;
    
    if (!Json::parseFromStream(reader_builder, file, &json, &errors)) {
        throw std::runtime_error("JSON parse error in " + filepath + ": " + errors);
    }
    
    return json;
}

int main(int argc, char* argv[]) {
    CLI::App app("UART JSON Client - Send JSON messages over serial port");
    
    std::string port = "/dev/ttyACM0";
    unsigned int baud = 9600;
    std::string parity_str = "NONE";
    unsigned int data_bits = 8;
    std::string stop_bits_str = "1";
    std::string request_file;
    
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
    
    app.add_option("--request-file", request_file, "Path to JSON file containing request message");
    
    CLI11_PARSE(app, argc, argv);
    
    // Parse serial parameters
    SerialParity parity = parse_parity(parity_str);
    SerialDataBits db = parse_data_bits(data_bits);
    SerialStopBits sb = parse_stop_bits(stop_bits_str);
    
    // Read request message
    Json::Value request;
    if (!request_file.empty()) {
        request = read_json_from_file(request_file);
    } else {
        request["type"] = "test_request";
        request["payload"] = "Hello, world!";
    }
    std::cout << "Sending request: " << request.toStyledString() << std::endl;
    
    // Open serial port
    serialib uart;
    const auto open_result = uart.openDevice(
        port.c_str(),
        baud,
        db,
        parity,
        sb
    );
    if (open_result != 1) {
        std::cerr << "Failed to open serial port: " << port << std::endl;
        return 1;
    }
    
    // Setup frame readers/writers
    LinuxUartCobsFrameReader<BUFF_SIZE> cobs_reader(&uart);
    LinuxUartCobsFrameWriter cobs_writer(&uart);
    JsonMessageReader json_reader(&cobs_reader);
    JsonMessageWriter json_writer(&cobs_writer);
    
    // Send request
    uart.flushReceiver();
    json_writer.write(request);
    
    // Read response
    while (true) {
        const auto response = json_reader.read();
        if (!response.has_value()) {
            continue;
        }
        const auto msg = response.value();
        std::cout << "Received response: " << msg.toStyledString() << std::endl;
        break;
    }
    
    uart.closeDevice();
    return 0;
}