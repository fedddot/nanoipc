# nanoipc

## Purpose

Implementing a reliable IPC (inter-process communication) layer between a host machine and an MCU from scratch in every embedded project is repetitive and error-prone. Existing solutions are either too heavy for constrained devices or are tightly coupled to a specific application-layer protocol (request/response, pub-sub, async, etc.), forcing architectural decisions on the developer.

**nanoipc** solves this by providing a minimal, platform-independent C++17 building block for message exchange over byte-stream transports such as UART. It is deliberately protocol-agnostic: the library handles only serialization and framing, leaving concurrency, sequencing, and application-layer concerns entirely to the application. Any protocol—synchronous request/response, asynchronous fire-and-forget, pub-sub—can be built on top without modification to the library itself.

The library uses [nanopb](https://github.com/nanopb/nanopb) (Protocol Buffers) and [JsonCPP](https://github.com/open-source-parsers/jsoncpp) for message encoding and [nanocobs](https://github.com/charlesnicholson/nanocobs) (COBS framing) for reliable message delimiting on byte streams.

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
│   ├── linux/
│   │   └── include/
│   │       ├── linux_uart_cobs_frame_reader.hpp  # Linux UART COBS frame reader
│   │       └── linux_uart_cobs_frame_writer.hpp  # Linux UART COBS frame writer
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
├── apps/                       # Host applications
│   └── linux_uart_json_client/ # CLI client for sending JSON over UART
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
   - LinuxUartCobsFrameReader/Writer: Linux UART-backed frame I/O using serialib
5. **Message Layer** - Protocol Buffer and JSON encoding/decoding
   - PbMessageReader/Writer: serialize/deserialize protobuf messages
   - JsonMessageReader/Writer: serialize/deserialize JSON messages using JsonCPP

## Typical usage

### Server side (MCU)

The server runs on a microcontroller. It continuously reads raw bytes from a transport (e.g. UART), accumulates them in a `RingBuffer`, and passes them to a `CobsFrameReader` to decode complete COBS frames. Each frame is then deserialized into an API message by a `JsonMessageReader` or `PbMessageReader`. After processing the request, the server encodes the response, wraps it in a COBS frame with `CobsFrameWriter`, and sends it back over the transport.

```
UART bytes → RingBuffer → CobsFrameReader → JsonMessageReader → [app logic]
                                                                       ↓
UART bytes ← CobsFrameWriter ← JsonMessageWriter ← [app logic response]
```

### Client side (host / another MCU)

The client serializes a request via a `Writer`, wraps it in a COBS frame, and sends it over the transport. It then reads raw response bytes, decodes the COBS frame, and deserializes the message with a `Reader`.

```
[app logic] → JsonMessageWriter → CobsFrameWriter → UART
[app logic] ← JsonMessageReader ← CobsFrameReader ← UART bytes
```

The `Reader`/`Writer` separation is intentional. nanoipc does **not** implement any application-layer protocol (no request-ID matching, no timeouts, no pub-sub brokering). This keeps the library MCU-friendly and lets you build any protocol on top: synchronous blocking request/response, fully asynchronous pipelines, pub-sub, and so on.

## Integration

### With CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    nanoipc
    GIT_REPOSITORY https://github.com/fedddot/nanoipc.git
    GIT_TAG        main   # pin to a specific tag or commit for reproducible builds
)

# Optional: disable what you don't need before making it available
set(NANOIPC_TESTS OFF)
set(NANOIPC_LINUX_FEATURES OFF)   # set ON only on Linux host builds

FetchContent_MakeAvailable(nanoipc)
```

Then link the targets you need:

```cmake
target_link_libraries(my_target
    PRIVATE
    cobs_frame_reader   # COBS framing – reader side
    cobs_frame_writer   # COBS framing – writer side
    json_message_reader # JSON message deserialization
    json_message_writer # JSON message serialization
    # pb_message_reader / pb_message_writer for Protocol Buffers instead
)
```

### With add_subdirectory

Clone or copy the repository into your project tree, then:

```cmake
set(NANOIPC_TESTS OFF)
set(NANOIPC_LINUX_FEATURES OFF)

add_subdirectory(nanoipc)

target_link_libraries(my_target PRIVATE cobs_frame_reader json_message_writer)
```

### CMake options

| Option | Default | Description |
|---|---|---|
| `NANOIPC_TESTS` | `ON` | Build the GoogleTest-based test suite. Disable for cross-compiled / resource-constrained builds. |
| `NANOIPC_LINUX_FEATURES` | `ON` | Build Linux-specific implementations (`LinuxUartCobsFrameReader/Writer`). Disable when targeting bare-metal MCUs. |

## Running the tests (host)

```bash
cmake -B build-tests
cmake --build build-tests
ctest --test-dir build-tests
```
