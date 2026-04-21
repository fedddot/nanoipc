# nanoipc

A minimal C++17 IPC library for embedded systems using [nanopb](https://github.com/nanopb/nanopb) (Protocol Buffers) and [nanocobs](https://github.com/charlesnicholson/nanocobs) (COBS framing) over a byte-stream transport such as UART.

## Repository layout

```
nanoipc/
├── nanoipc_read_buffer.hpp   # ReadBuffer interface
├── nanoipc_reader.hpp        # NanoIpcReader – deserialises incoming messages
├── nanoipc_writer.hpp        # NanoIpcWriter – serialises outgoing messages
├── nanoipc_utils/            # COBS encode/decode helpers + NanoipcRingBuffer
├── nanopb_utils/             # nanopb parser and serialiser adapters
├── tests/                    # Google Test suite (host build)
└── example/
    ├── api/                  # Shared API: example.proto, typed reader/writer classes
    ├── example_client/       # Host-side client (Linux/macOS, reads/writes a serial fd)
    └── example_server/       # Raspberry Pi Pico firmware (see below)
```

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
