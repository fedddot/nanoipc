#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "json_message_reader.hpp"
#include "json_message_writer.hpp"
#include "uart_connection.hpp"
#include "ring_buffer.hpp"
#include "json/value.h"

using namespace nanoipc;

int main() {
    RingBuffer<512> ring_buffer;
    UartConnection uart(
        "/dev/ttyACM0",
        9600,
        SERIAL_PARITY_NONE,
        SERIAL_STOPBITS_1,
        SERIAL_DATABITS_8,
        [&ring_buffer](uint8_t c) {
            ring_buffer.push_back(c);
        }
    );
    CobsFrameReader cobs_reader(&ring_buffer);
    CobsFrameWriter cobs_writer(
        [&uart](const uint8_t* data, size_t length) {
            uart.write(data, length);
        }
    );
    JsonMessageReader json_reader(&cobs_reader);
    JsonMessageWriter json_writer(&cobs_writer);

    uart.open();

    Json::Value message;
    message["type"] = "greeting";
    message["content"] = "Hello from the JSON client!";
    json_writer.write(message);
    while (true) {
        const auto received_message = json_reader.read();
        if (!received_message.has_value()) {
            continue;
        }
        const auto& msg = received_message.value();
        std::cout << "Received message: " << msg.toStyledString() << std::endl;
        break;
    }
    uart.close();

   return 0;
}