/// @file example_server.cpp
/// @brief NanoIPC example server for Raspberry Pi Pico (RP2040).
///
/// Listens for ExampleRequest protobuf messages on UART0 (GP0 TX / GP1 RX),
/// processes each request, and replies with an ExampleResponse message.

// ─── Pico SDK ─────────────────────────────────────────────────────────────────
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

// ─── Standard headers ────────────────────────────────────────────────────────
#include <cstddef>
#include <cstdint>

// ─── NanoIPC ─────────────────────────────────────────────────────────────────
#include "nanoipc_ring_buffer.hpp"

// ─── API ─────────────────────────────────────────────────────────────────────
#include "example_api.hpp"
#include "example_request_reader.hpp"
#include "example_response_writer.hpp"

// =============================================================================
// Constants
// =============================================================================

namespace {
    uart_inst_t* const UART_ID = uart0;
    constexpr uint         BAUD    = 9600;
    constexpr uint         TX_PIN  = 0;  // GP0
    constexpr uint         RX_PIN  = 1;  // GP1
}

// =============================================================================
// Globals
// =============================================================================

/// Buffer filled byte-by-byte from the UART Rx IRQ handler.
static nanoipc::NanoipcRingBuffer<256> g_read_buffer;

// =============================================================================
// UART implementation (UART0, GP0 TX / GP1 RX)
// =============================================================================

/// @brief UART0 RX IRQ handler – push each received byte into the read buffer.
static void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        g_read_buffer.push_back(static_cast<uint8_t>(uart_getc(UART_ID)));
    }
}

/// @brief Initialise UART0 and register the RX interrupt handler.
static void uart_setup() {
    uart_init(UART_ID, BAUD);
    gpio_set_function(TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);
    uart_set_baudrate(UART_ID, BAUD);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
}

/// @brief Transmit one byte over UART0, blocking until accepted by the FIFO.
static void uart_write_byte(const uint8_t byte) {
    uart_putc(UART_ID, static_cast<char>(byte));
}

// =============================================================================
// Business logic
// =============================================================================

/// Process a single request and return the corresponding response.
static api::ExampleResponse process_request(const api::ExampleRequest& req) {
    switch (req.action) {
        case api::Action::ADD:
            return api::ExampleResponse{ .result = req.value + 1 };
        case api::Action::SUBTRACT:
            return api::ExampleResponse{ .result = req.value - 1 };
        default:
            return api::ExampleResponse{ .result = req.value };
    }
}

// =============================================================================
// Application entry points
// =============================================================================

static api::ExampleRequestReader  *g_reader = nullptr;
static api::ExampleResponseWriter *g_writer = nullptr;

/// Static storage for reader/writer instances (avoids heap allocation).
static uint8_t g_reader_storage[sizeof(api::ExampleRequestReader)];
static uint8_t g_writer_storage[sizeof(api::ExampleResponseWriter)];

/// Initialise UART and construct the reader/writer.
static void setup() {
    stdio_init_all();
    uart_setup();

    g_reader = new (g_reader_storage) api::ExampleRequestReader(&g_read_buffer);

    g_writer = new (g_writer_storage) api::ExampleResponseWriter(
        [](const uint8_t* data, const std::size_t size) {
            for (std::size_t i = 0; i < size; ++i) {
                uart_write_byte(data[i]);
            }
        }
    );
}

/// Main loop – poll the reader; when a complete request arrives, process it
/// and send the response.
static void loop() {
    if (!g_reader || !g_writer) {
        return;
    }

    const auto request = g_reader->read();
    if (!request.has_value()) {
        return;
    }

    const auto response = process_request(request.value());
    g_writer->write(response);
}

// =============================================================================
// Main
// =============================================================================

int main() {
    setup();
    while (true) {
        loop();
    }
}
