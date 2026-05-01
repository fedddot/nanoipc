# UART Emulation on Linux — Design Note

## Context

The `frame/linux` library and related apps communicate over UART via `serialib`, which opens
`/dev/ttyXXX` devices. To enable automated testing in CI/CD without physical hardware, a
virtual serial-port pair is needed: two linked pseudo-devices where bytes written to one
appear on the other, just like a null-modem cable.

---

## Evaluated Approaches

### 1. socat

`socat` is a general-purpose relay that can connect two **pseudo-terminal (PTY)** devices
together. A single shell command creates a linked pair:

```sh
socat -d -d \
  pty,raw,echo=0,link=/tmp/ttyVIRT0 \
  pty,raw,echo=0,link=/tmp/ttyVIRT1 &
```

Both `/tmp/ttyVIRT0` and `/tmp/ttyVIRT1` become symlinks to real PTY devices that
`serialib` can open with the standard `openDevice()` call.

| Criterion | Assessment |
|---|---|
| Ease of setup | `apk add socat` — one line, no reboot |
| Looks like a real `/dev/ttyXXX` | ✅ PTY devices are indistinguishable from a real UART |
| Works inside Docker (no kernel module) | ✅ Purely user-space |
| Active maintenance | ✅ Widely packaged, actively maintained |

### 2. tty0tty

`tty0tty` is a Linux **kernel module** that registers a pair of null-modem virtual serial
ports (e.g. `/dev/tnt0` ↔ `/dev/tnt1`). The devices look exactly like physical hardware to
`termios`.

| Criterion | Assessment |
|---|---|
| Ease of setup | Requires kernel headers and `insmod`/`modprobe` |
| Looks like a real `/dev/ttyXXX` | ✅ Native TTY devices |
| Works inside Docker (no kernel module) | ❌ Cannot load kernel modules in a standard container |
| Active maintenance | ⚠️ Low activity; kernel-version sensitive |

### 3. Custom userspace PTY pair (`openpty` / `posix_openpt`)

A test fixture (or a small helper process) calls `openpty()` (a BSD/Linux extension) or
the fully POSIX `posix_openpt()` + `grantpt()` + `unlockpt()` + `ptsname_r()` sequence to
allocate a PTY master/slave pair in C/C++, without any external tool. The test code holds
the master fd and passes the slave device path to the code under test.

> **Note:** PTY-based tests validate `serialib` device-open and byte-stream framing. They
> do **not** emulate physical-UART signaling (modem-control lines, parity/framing errors,
> disconnect events, etc.).

| Criterion | Assessment |
|---|---|
| Ease of setup | No extra packages — provided by the system C library |
| Looks like a real `/dev/ttyXXX` | ✅ Slave side is `/dev/pts/N`, opened by `serialib` |
| Works inside Docker (no kernel module) | ✅ Purely user-space; needs only `/dev/ptmx` + `devpts` |
| Active maintenance | N/A — part of the platform C library (musl on Alpine, glibc on Ubuntu) |

---

## Chosen Approach — Custom PTY pair (`openpty` / `posix_openpt`)

**Recommendation:** use a **custom userspace PTY pair** as the primary mechanism for unit
tests, with `socat` available in the dev container for integration/manual testing.

### Rationale

1. **No external tool required for unit tests.** `openpty` is available on all Linux
   platforms: it is provided by **musl libc** on Alpine and by **glibc** (`-lutil`) on
   Ubuntu/Debian. No package installation is required and there is no risk of a missing
   tool breaking the build.

2. **Sufficient for `LinuxUartCobsFrameReader` / `LinuxUartCobsFrameWriter`.** Both classes
   depend only on `serialib` byte I/O (`readChar`, `writeBytes`, `available`) and a normal
   termios-backed device open. A PTY slave satisfies all of those requirements.

3. **Fast and deterministic.** The PTY is created synchronously inside the test process.
   There is no race condition waiting for an external `socat` process to start and write
   its symlinks to disk.

4. **Easily integrated with GoogleTest.** A test fixture can open the PTY pair in `SetUp()`,
   pass the slave path to the class under test, and hold the master fd for injection — all
   without spawning subprocesses.

5. **`socat` remains valuable** for interactive debugging, manual smoke-testing, and
   potential future integration tests that involve running a full app binary.

`tty0tty` is ruled out because kernel-module loading is incompatible with standard Docker
containers.

---

## How to Use It

### Shared fixture: `VirtualUart`

All tests — both unit and integration — use a single GoogleTest fixture defined in
`virtual_uart.hpp`. The header is delivered as a CMake **INTERFACE** target named
`virtual_uart` so any test executable can depend on it without adding source files to its
own target.

```
testing/
  virtual_uart/
    CMakeLists.txt          # INTERFACE target
    include/
      virtual_uart.hpp      # fixture definition
```

The fixture manages the PTY pair lifecycle and exposes raw byte I/O on the master side:

```cpp
#ifndef VIRTUAL_UART_HPP
#define VIRTUAL_UART_HPP

#include <pty.h>
#include <unistd.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

class VirtualUart : public ::testing::Test {
public:
    // Returns the slave device path (e.g. /dev/pts/N).
    // Pass this to serialib::openDevice() or as --port to the app under test.
    const std::string& slave_path() const { return m_slave_path; }

    // Write raw bytes to the master side of the PTY
    // (simulates data arriving from the remote device).
    void write_data(const std::vector<std::uint8_t>& data) {
        const auto written = ::write(m_master_fd, data.data(), data.size());
        if (written < 0 || static_cast<std::size_t>(written) != data.size()) {
            throw std::runtime_error("write_data: write failed");
        }
    }

    // Read exactly n raw bytes from the master side of the PTY
    // (bytes that the code under test / app sent to the slave).
    std::vector<std::uint8_t> read_data(std::size_t n) {
        std::vector<std::uint8_t> buf(n);
        std::size_t total = 0;
        while (total < n) {
            const auto got = ::read(m_master_fd, buf.data() + total, n - total);
            if (got <= 0) { throw std::runtime_error("read_data: read failed"); }
            total += static_cast<std::size_t>(got);
        }
        return buf;
    }

protected:
    void SetUp() override {
        char name[64]{};
        // Pass nullptr for termios/winsize — serialib reconfigures them on openDevice().
        if (openpty(&m_master_fd, &m_slave_fd, name, nullptr, nullptr) != 0) {
            throw std::runtime_error("SetUp: openpty failed");
        }
        m_slave_path = name;
    }

    void TearDown() override {
        if (m_slave_fd  >= 0) { ::close(m_slave_fd);  m_slave_fd  = -1; }
        if (m_master_fd >= 0) { ::close(m_master_fd); m_master_fd = -1; }
    }

private:
    int m_master_fd{-1};
    int m_slave_fd{-1};
    std::string m_slave_path;
};

#endif // VIRTUAL_UART_HPP
```

CMake INTERFACE target (no sources — the fixture is header-only):

```cmake
# testing/virtual_uart/CMakeLists.txt
add_library(virtual_uart INTERFACE)
target_include_directories(virtual_uart INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(virtual_uart INTERFACE gtest gtest_main)
```

`openpty` is in `<pty.h>` on both musl (Alpine) and glibc; on glibc-based systems
(Ubuntu/Debian) link the test executable with `-lutil`.

---

### Unit tests (`frame/linux`)

Each test case inherits the PTY pair from `VirtualUart` via `TEST_F`. The test opens the
slave device with `serialib`, exercises `LinuxUartCobsFrameReader` /
`LinuxUartCobsFrameWriter`, and uses `write_data()` / `read_data()` to drive the master
side:

```cmake
# frame/linux/tests/CMakeLists.txt
add_executable(linux_uart_frame_tests
    src/ut_linux_uart_cobs_frame.cpp
)
target_link_libraries(linux_uart_frame_tests
    PRIVATE
    virtual_uart
    linux_uart_cobs_frame_reader
    linux_uart_cobs_frame_writer
)
add_test(NAME linux_uart_frame_tests COMMAND linux_uart_frame_tests)
```

```cpp
// ut_linux_uart_cobs_frame.cpp
#include "virtual_uart.hpp"
#include "linux_uart_cobs_frame_reader.hpp"
#include "linux_uart_cobs_frame_writer.hpp"
#include "serialib.h"

TEST_F(VirtualUart, WriterSendsCobsFrame) {
    serialib uart;
    ASSERT_EQ(uart.openDevice(slave_path().c_str(), 9600), 1);

    nanoipc::LinuxUartCobsFrameWriter writer(&uart);
    writer.write({0x01, 0x02, 0x03});

    // read_data receives raw COBS bytes from the master side
    const auto raw = read_data(5); // 5 = COBS-encoded length of {01 02 03}
    // assert expected COBS encoding ...
    uart.closeDevice();
}
```

---

### Integration tests (`apps/`)

Integration tests spawn the real app binary as a subprocess and use the `VirtualUart`
fixture to act as the **device side** on the master fd.

#### CMake structure

The app target is already defined by the parent CMakeLists.txt (via
`apps/CMakeLists.txt`). The integration-test CMakeLists.txt creates its own executable and
uses `add_dependencies` to guarantee the app binary is built before the tests run — no
`add_subdirectory` of the app is needed:

```cmake
# apps/linux_uart_cobs_client/tests/CMakeLists.txt
add_executable(linux_uart_cobs_client_it
    src/it_linux_uart_cobs_client.cpp
)
target_link_libraries(linux_uart_cobs_client_it
    PRIVATE
    virtual_uart
    cobs_frame_reader
    cobs_frame_writer
)

# Ensure the app binary is built before the test runs.
add_dependencies(linux_uart_cobs_client_it linux_uart_cobs_client)

# Inject the path to the built binary at compile time.
target_compile_definitions(linux_uart_cobs_client_it PRIVATE
    APP_BINARY="$<TARGET_FILE:linux_uart_cobs_client>"
)

add_test(NAME linux_uart_cobs_client_it COMMAND linux_uart_cobs_client_it)
```

#### Test body

Each test case uses `TEST_F(VirtualUart, ...)`. The fixture provides `slave_path()`,
`write_data()`, and `read_data()`; the test body is responsible for spawning the
subprocess and coordinating the exchange:

```cpp
// it_linux_uart_cobs_client.cpp
#include <sys/wait.h>
#include <unistd.h>
#include "virtual_uart.hpp"
// COBS helpers reused from the library under test
#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"

TEST_F(VirtualUart, SendsRequestAndReceivesResponse) {
    // --- prepare test data ---
    const std::vector<std::uint8_t> request_payload  = {0xDE, 0xAD, 0xBE, 0xEF};
    const std::vector<std::uint8_t> response_payload = {0xCA, 0xFE};
    // write request binary to a temp file
    const std::string req_file = "/tmp/cobs_it_request.bin";
    const std::string rsp_file = "/tmp/cobs_it_response.bin";
    // ... write request_payload to req_file ...

    // --- spawn the application ---
    const pid_t pid = fork();
    ASSERT_GE(pid, 0);
    if (pid == 0) {
        // child: exec the app pointing at the slave PTY
        execl(APP_BINARY, APP_BINARY,
              "--port",     slave_path().c_str(),
              "--baud",     "9600",
              "--request",  req_file.c_str(),
              "--response", rsp_file.c_str(),
              nullptr);
        _exit(127);
    }

    // --- act as the device on the master side ---
    // Read the COBS frame the app sent, then write back a COBS response.
    // write_data / read_data provide raw byte access to the master fd.
    // Use CobsFrameWriter / CobsFrameReader (linked via CMake) for framing.
    // ... decode incoming frame, verify it matches request_payload ...
    // ... encode response_payload as COBS and call write_data() ...

    // --- wait for the app to exit ---
    int status{};
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), 0);

    // --- assert response file content ---
    // ... read rsp_file, compare with response_payload ...
}
```

#### Interaction flow

```
Test process (VirtualUart fixture)      App subprocess
────────────────────────────────────    ─────────────────────────────────
SetUp(): openpty → slave_path()
fork() → spawn app with
  --port slave_path()
  --request req_file
  --response rsp_file             ─────► openDevice(slave_path())
                                          flushReceiver()
                                          cobs_writer.write(request)
read_data() → decode COBS frame  ◄──────  slave: COBS bytes sent
write_data(COBS-encoded response) ──────► slave: COBS bytes received
                                          cobs_reader.read() → payload
                                          write rsp_file → exit 0
waitpid() → assert exit code 0
read rsp_file → assert content
TearDown(): close PTY fds
```

### Dev container (`docker/dev.dockerfile`)

Add `socat` to the Alpine package installation line for interactive debugging:

```dockerfile
RUN apk add git make gcc g++ gdb cmake clang-extra-tools valgrind socat
```

No additional packages are needed for unit tests. On Alpine, `<pty.h>` and `openpty()` are
provided by musl libc, which is already present.

### CI/CD

No changes to the CI pipeline are required. Because the PTY pair is created in-process,
tests run with the same `cmake -B build && cmake --build build && ctest` commands already
used today.
