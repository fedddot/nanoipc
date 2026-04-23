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
├── message/                    # Protocol Buffer message layer
│   ├── include/
│   │   ├── pb_message_reader.hpp  # Protocol Buffer deserialization
│   │   └── pb_message_writer.hpp  # Protocol Buffer serialization
│   └── tests/
│       └── src/
│           └── ut_pb_message.cpp     # Protocol Buffer tests
├── CMakeLists.txt              # Build configuration
├── tests/                      # Integration tests (host build)
└── example/
    ├── api/                    # Shared API: example.proto, generated message types
    ├── example_client/         # Host-side client (Linux/macOS, reads/writes a serial port)
    └── example_server/         # Raspberry Pi Pico firmware
```

## Architecture

nanoipc uses a layered architecture:

1. **Reader/Writer Interfaces** - Generic templates for reading/writing typed data
2. **ReadBuffer** - Abstract interface for buffering incoming bytes
3. **RingBuffer** - Concrete implementation of ReadBuffer (fixed-capacity circular buffer)
4. **Frame Layer** - COBS encoding/decoding for delimiting messages on byte streams
   - CobsFrameReader/Writer: serialize/deserialize frames
5. **Message Layer** - Protocol Buffer encoding/decoding
   - PbMessageReader/Writer: serialize/deserialize protobuf messages

## Example server (Raspberry Pi Pico – RP2040)

The example server runs on the [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) (RP2040, dual-core Cortex-M0+, 2 MiB flash, 264 KiB RAM).

It receives `ExampleRequest` protobuf messages over **UART0** (GP0 TX / GP1 RX at 9600 baud), processes them, and responds with `ExampleResponse` messages over the same port.

### Prerequisites

| Tool | Install (Debian / Ubuntu) |
|------|--------------------------|
| CMake ≥ 3.16 | `sudo apt install cmake` |
| ARM GCC cross-compiler | `sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi` |
| Python 3 (nanopb generator) | `sudo apt install python3` |
| Git | `sudo apt install git` |

The Pico SDK is fetched automatically by CMake at configure time (`PICO_SDK_FETCH_FROM_GIT ON`). No separate SDK installation is required.

### Build

```bash
cd example/example_server

cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel

cmake --build build
```

The build produces the following artefacts inside `build/`:

| File | Description |
|------|-------------|
| `example_server.elf` | ELF image (use with a debugger / OpenOCD) |
| `example_server.uf2` | UF2 image – drag-and-drop onto Pico in BOOTSEL mode |
| `example_server.bin` | Raw binary |
| `example_server.hex` | Intel HEX |

### Flash

**UF2 (easiest):**
Hold the BOOTSEL button while plugging in the Pico. It mounts as a USB mass-storage device; copy the UF2 file onto it:
```bash
cp build/example_server.uf2 /media/$USER/RPI-RP2/
```

**OpenOCD (SWD, via Picoprobe or another Pico running the debugprobe firmware):**
```bash
openocd -f interface/cmsis-dap.cfg \
        -f target/rp2040.cfg \
        -c "program build/example_server.elf verify reset exit"
```

### UART0 pin-out

| Pico pin | GPIO | Signal | Connect to |
|----------|------|--------|------------|
| Pin 1 | GP0 | TX | USB-UART adapter RX |
| Pin 2 | GP1 | RX | USB-UART adapter TX |
| Pin 3 | GND | GND | USB-UART adapter GND |

Settings: **9600 baud, 8N1**.

## Example client (host)

The example client runs on a Linux/macOS host. It opens a serial port, sends `ExampleRequest` messages, and prints the received `ExampleResponse` messages.

### Build

```bash
cd example/example_client

cmake -B build

cmake --build build
```

### Run

```bash
build/example_client /dev/ttyUSB0
```

## Running the tests (host)

```bash
cmake -B build-tests
cmake --build build-tests
ctest --test-dir build-tests
```

