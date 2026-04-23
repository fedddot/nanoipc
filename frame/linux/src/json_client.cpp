#include <cstddef>

#include "linux_uart_cobs_frame_reader.hpp"
#include "linux_uart_cobs_frame_writer.hpp"
#include "json_message_reader.hpp"
#include "json_message_writer.hpp"
#include "serialib.h"
#include "json/value.h"

using namespace nanoipc;

static unsigned int get_baud(int argc, char* argv[]);
static SerialParity get_parity(int argc, char* argv[]);
static SerialDataBits get_data_bits(int argc, char* argv[]);
static SerialStopBits get_stop_bits(int argc, char* argv[]);
static std::string get_serial_port(int argc, char* argv[]);
static Json::Value read_json_message(int argc, char* argv[]);

enum: std::size_t { BUFF_SIZE = 1024UL };

int main(int argc, char* argv[]) {
    serialib uart;
    const auto open_result = uart.openDevice(
        get_serial_port(argc, argv).c_str(),
        get_baud(argc, argv),
        get_data_bits(argc, argv),
        get_parity(argc, argv),
        get_stop_bits(argc, argv)
    );
    if (open_result != 1) {
        std::cerr << "Failed to open serial port: " << get_serial_port(argc, argv) << std::endl;
        return 1;
    }
    LinuxUartCobsFrameReader<BUFF_SIZE> cobs_reader(&uart);
    LinuxUartCobsFrameWriter cobs_writer(&uart);
    JsonMessageReader json_reader(&cobs_reader);
    JsonMessageWriter json_writer(&cobs_writer);

    const auto request = read_json_message(argc, argv);
    json_writer.write(request);
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

unsigned int get_baud(int argc, char* argv[]) {
    return 9600; // TODO: retrieve from command line arguments (--baud)
}
SerialParity get_parity(int argc, char* argv[]) {
    return SerialParity::SERIAL_PARITY_NONE; // TODO: retrieve from command line arguments (--parity)
}
SerialDataBits get_data_bits(int argc, char* argv[]) {
    return SerialDataBits::SERIAL_DATABITS_8; // TODO: retrieve from command line arguments (--data-bits)
}
SerialStopBits get_stop_bits(int argc, char* argv[]) {
    return SerialStopBits::SERIAL_STOPBITS_1; // TODO: retrieve from command line arguments (--stop-bits)
}
std::string get_serial_port(int argc, char* argv[]) {
    return "/dev/ttyACM0"; // TODO: retrieve from command line arguments (--port)
}
Json::Value read_json_message(int argc, char* argv[]) {
    Json::Value request;
    request["type"] = "test_request";
    request["payload"] = "Hello, world!";
    return request; // TODO: retrieve from command line arguments (--request-file)
}