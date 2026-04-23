# nanoipc

A minimal C++17 IPC library for embedded systems using [nanopb](https://github.com/nanopb/nanopb) (Protocol Buffers) and [nanocobs](https://github.com/charlesnicholson/nanocobs) (COBS framing) over a byte-stream transport such as UART.

## Repository layout

```
nanoipc/
├── include/                    # Core interfaces
│   ├── reader.hpp             # Reader<T> template interface
│   └── writer.hpp             # Writer<T> template interface
├── frame/                      # COBS frame layer
│   ├── include/
│   │   ├── read_buffer.hpp    # ReadBuffer interface for byte storage
│   │   ├── ring_buffer.hpp    # Fixed-capacity circular buffer
│   │   ├── cobs_frame_reader.hpp  # COBS frame deserialization
│   │   └── cobs_frame_writer.hpp  # COBS frame serialization
│   └── tests/
│       └── src/
│           ├── ut_cobs_frame.cpp       # COBS frame tests
│           └── ut_ring_buffer.cpp      # Ring buffer tests
├── message/                    # Message layer (Protocol Buffer & JSON)
│   ├── include/
│   │   ├── pb_message_reader.hpp  # Protocol Buffer deserialization
│   │   ├── pb_message_writer.hpp  # Protocol Buffer serialization
│   │   ├── json_message_reader.hpp    # JSON deserialization
│   │   └── json_message_writer.hpp    # JSON serialization
│   └── tests/
│       └── src/
│           ├── ut_pb_message.cpp     # Protocol Buffer tests
│           └── ut_json_message.cpp   # JSON message tests
├── utils/                       # Utility classes for embedded systems
│   ├── include/
│   │   └── uart_connection.hpp  # Linux UART connection management
│   ├── src/
│   │   └── uart_connection.cpp  # UART connection implementation
│   └── tests/
│       └── src/
│           └── ut_uart_connection.cpp # UART connection tests
├── CMakeLists.txt              # Build configuration
└── tests/                      # Integration tests (host build)
```

## Architecture

nanoipc uses a layered architecture:

1. **Reader/Writer Interfaces** - Generic templates for reading/writing typed data
2. **ReadBuffer** - Abstract interface for buffering incoming bytes
3. **RingBuffer** - Concrete implementation of ReadBuffer (fixed-capacity circular buffer)
4. **Frame Layer** - COBS encoding/decoding for delimiting messages on byte streams
   - CobsFrameReader/Writer: serialize/deserialize frames
5. **Message Layer** - Protocol Buffer and JSON encoding/decoding
   - PbMessageReader/Writer: serialize/deserialize protobuf messages
   - JsonMessageReader/Writer: serialize/deserialize JSON messages using JsonCPP
6. **Utils Layer** - Utility classes for embedded systems
   - UartConnection: Linux UART connection management with serialib

## UartConnection Usage

The `UartConnection` class manages Linux UART connections with automatic background listening for incoming data.

### Basic Example

```cpp
#include "uart_connection.hpp"
#include <iostream>

int main() {
    // Define a callback for received characters
    auto on_char_received = [](uint8_t c) {
        std::cout << "Received: " << (int)c << std::endl;
    };

    // Create a UART connection using serialib enums
    nanoipc::UartConnection uart(
        "/dev/ttyUSB0",           // Device path
        115200,                   // Baudrate
        SERIAL_PARITY_NONE,       // Parity (SERIAL_PARITY_NONE, SERIAL_PARITY_ODD, SERIAL_PARITY_EVEN)
        SERIAL_STOPBITS_1,        // Stop bits (SERIAL_STOPBITS_1 or SERIAL_STOPBITS_2)
        SERIAL_DATABITS_8,        // Data bits (SERIAL_DATABITS_5 through SERIAL_DATABITS_8)
        on_char_received          // Callback for received characters
    );

    // Open the connection (starts background listening thread)
    uart.open();

    // Write data
    const uint8_t data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    uart.write(data, sizeof(data));

    // ... do other work while listening for data in the background ...

    // Close the connection
    uart.close();

    return 0;
}
```

### Key Features

- **Non-blocking I/O**: The `open()` method starts a background thread for listening, allowing non-blocking communication
- **Type-Safe Enums**: Uses serialib's native enum types (SerialParity, SerialStopBits, SerialDataBits) for compile-time safety
- **Exception-based Error Handling**: All methods throw exceptions on errors
- **Thread-Safe**: Uses atomic flags for thread synchronization
- **Callback-Based**: Invokes a callback function for each received character
- **Serialib Integration**: Uses the serialib library for low-level UART access

## Running the tests (host)

```bash
cmake -B build-tests
cmake --build build-tests
ctest --test-dir build-tests
```

