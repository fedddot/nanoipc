# linux_uart_cobs_client

A command-line application that acts as a COBS/UART framing bridge for host-side clients written in languages other than C++. It reads a raw binary payload from a file, wraps it in a COBS frame, sends it over a UART serial port, receives a COBS-framed response, strips the framing, and writes the raw binary payload to an output file.

This makes it useful as a thin transport helper for any language that has its own serialisation stack (e.g. Rust with `prost` for Protobuf). The application performs no JSON serialisation or deserialisation.

## Building

The app is part of the top-level nanoipc CMake project and is only compiled when the `NANOIPC_LINUX_FEATURES` option is enabled (it is **on** by default).

```bash
# From the repository root
cmake -S . -B build -DNANOIPC_LINUX_FEATURES=ON -DNANOIPC_TESTS=OFF
cmake --build build --target linux_uart_cobs_client
```

The resulting binary is placed in `build/apps/linux_uart_cobs_client/`.

## Running

```
linux_uart_cobs_client [OPTIONS]
```

| Option | Default | Description |
|---|---|---|
| `--port` | `/dev/ttyACM0` | Serial device path (e.g. `/dev/ttyUSB0`) |
| `--baud` | `9600` | Baud rate (e.g. `115200`, `230400`) |
| `--parity` | `NONE` | Parity: `NONE`, `EVEN`, `ODD`, `MARK`, `SPACE` |
| `--data-bits` | `8` | Data bits: `5`, `6`, `7`, `8`, `16` |
| `--stop-bits` | `1` | Stop bits: `1`, `1.5`, `2` |
| `--request` | *(required)* | Path to binary file containing the raw request payload |
| `--response` | *(required)* | Path to write the raw binary response payload |
| `--timeout` | `3` | Timeout in seconds to wait for a response |

### Example – Rust client sending Protobuf over UART

```bash
# 1. Rust serialises a Protobuf request and writes it to a file
#    (done inside the Rust binary)

# 2. Shell out to linux_uart_cobs_client to handle COBS/UART framing
./linux_uart_cobs_client \
    --port /dev/ttyUSB0 \
    --baud 115200 \
    --request request.bin \
    --response response.bin \
    --timeout 5

# 3. Rust reads response.bin and deserialises the Protobuf response
#    (done inside the Rust binary)
```

**Expected output:**

```
Sending 32 byte(s) from request.bin
Received 48 byte(s), writing to response.bin
```

If the response is not received within the `--timeout` duration, the application prints an error and exits with a non-zero status code.
