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

## Running the tests (host)

```bash
cmake -B build-tests
cmake --build build-tests
ctest --test-dir build-tests
```

