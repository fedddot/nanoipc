/// @file example_server.cpp
/// @brief NanoIPC example server for STM32F103C8T6 (Blue Pill).
///
/// Listens for ExampleRequest protobuf messages on USART1 (PA9 TX / PA10 RX),
/// processes each request, and replies with an ExampleResponse message.
/// System clock is 72 MHz (HSE 8 MHz × PLL × 9), configured by startup_stm32f103.c.

// ─── Standard headers ────────────────────────────────────────────────────────
#include <cstddef>
#include <cstdint>

// ─── STM32 CMSIS device header ────────────────────────────────────────────────
#include "stm32f1xx.h"

// ─── NanoIPC ─────────────────────────────────────────────────────────────────
#include "nanoipc_reader.hpp"
#include "nanoipc_writer.hpp"
#include "nanopb_parser.hpp"
#include "nanopb_serializer.hpp"

// ─── Generated protobuf stubs (produced by nanopb_generator.py) ──────────────
#include "example.pb.h"

// ─── Local helpers ───────────────────────────────────────────────────────────
#include "uart_read_buffer.hpp"

// =============================================================================
// Domain types
// =============================================================================

#include "../api/example_request.hpp"
#include "../api/example_response.hpp"
#include "../api/example_request_reader.hpp"
#include "../api/example_response_writer.hpp"


// =============================================================================
// Globals
// =============================================================================

/// Buffer filled byte-by-byte from the UART Rx ISR.
static example::UartReadBuffer<256> g_read_buffer;

// =============================================================================
// UART implementation (USART1, PA9 TX / PA10 RX)
// =============================================================================

// APB2 clock = 72 MHz (set by SystemInit via HSE 8 MHz × PLL × 9).

/// @brief Initialise USART1.
static void uart_init(const uint32_t baud) {
    // Enable clocks for GPIOA and USART1 (both on APB2).
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    // PA9 = USART1 TX: alternate-function push-pull, 50 MHz.
    GPIOA->CRH &= ~(GPIO_CRH_CNF9  | GPIO_CRH_MODE9);
    GPIOA->CRH |=   GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0;

    // PA10 = USART1 RX: floating input (MODE = 00, CNF = 01).
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |=   GPIO_CRH_CNF10_0;

    // BRR = fPCLK2 / baud.
    USART1->BRR = static_cast<uint16_t>(72000000UL / baud);

    // Enable USART, transmitter, receiver, and RXNE interrupt.
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

    NVIC_EnableIRQ(USART1_IRQn);
}

/// @brief Transmit one byte, blocking until the Tx register is empty.
static void uart_write_byte(const uint8_t byte) {
    while (!(USART1->SR & USART_SR_TXE)) {}
    USART1->DR = byte;
}

/// @brief USART1 RXNE ISR – push the received byte into the read buffer.
/// C linkage matches the vector-table declaration in startup_stm32f103.c.
extern "C" void USART1_IRQHandler() {
    if (USART1->SR & USART_SR_RXNE) {
        g_read_buffer.push_back(static_cast<uint8_t>(USART1->DR & 0xFFU));
    }
}

// =============================================================================
// Protobuf transformers
// =============================================================================



// =============================================================================
// Business logic
// =============================================================================

/// Process a single request and return the corresponding response.
static ExampleResponse process_request(const ExampleRequest& req) {
    switch (req.action) {
        case Action::ADD:
            return ExampleResponse{ .result = req.value + 1 };
        case Action::SUBTRACT:
            return ExampleResponse{ .result = req.value - 1 };
        default:
            return ExampleResponse{ .result = req.value };
    }
}

// =============================================================================
// Application entry points
// =============================================================================

static nanoipc::NanoIpcReader<ExampleRequest>  *g_reader  = nullptr;
static nanoipc::NanoIpcWriter<ExampleResponse> *g_writer  = nullptr;

/// Static storage for reader/writer instances (avoids heap allocation).
static uint8_t g_reader_storage[sizeof(nanoipc::NanoIpcReader<ExampleRequest>)];
static uint8_t g_writer_storage[sizeof(nanoipc::NanoIpcWriter<ExampleResponse>)];

/// Initialise UART and construct the reader/writer.
void setup() {
    uart_init(9600);

    // --- Reader: parses incoming ExampleRequest messages ---
    g_reader = new (g_reader_storage) api::ExampleRequestReader(&g_read_buffer);

    // --- Writer: serialises outgoing ExampleResponse messages ---
    auto raw_writer = [](const uint8_t* data, const std::size_t size) {
        for (std::size_t i = 0; i < size; ++i) {
            uart_write_byte(data[i]);
        }
    };

    g_writer = new (g_writer_storage) api::ExampleResponseWriter(raw_writer);

}

/// Main loop – poll the reader; when a complete request arrives, process it
/// and send the response.
void loop() {
    if (!g_reader || !g_writer) {
        return;
    }

    const auto request = g_reader->read();
    if (!request.has_value()) {
        return; // no complete message yet
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
