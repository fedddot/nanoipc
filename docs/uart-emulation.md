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

### Unit tests (`frame/linux`)

A minimal test fixture pattern. `openpty` is in `<pty.h>` on both musl and glibc; on
glibc-based systems (Ubuntu/Debian) link with `-lutil`.

```cpp
#include <pty.h>       // openpty
#include <unistd.h>
#include <string>
#include <stdexcept>

struct PtyPair {
    int master_fd{-1};
    int slave_fd{-1};
    std::string slave_path;

    PtyPair() {
        char name[64]{};
        // Pass nullptr for termios/winsize — serialib reconfigures them on openDevice().
        if (openpty(&master_fd, &slave_fd, name, nullptr, nullptr) != 0) {
            throw std::runtime_error("openpty failed");
        }
        slave_path = name;
    }
    ~PtyPair() {
        if (slave_fd  >= 0) { close(slave_fd);  }
        if (master_fd >= 0) { close(master_fd); }
    }
    PtyPair(const PtyPair&) = delete;
    PtyPair& operator=(const PtyPair&) = delete;
};
```

The test then:
1. Constructs a `PtyPair`.
2. Opens `slave_path` with `serialib` (or passes it to the class under test).
3. Writes raw bytes to `master_fd` to simulate incoming UART data.
4. Reads bytes from `master_fd` to verify what the class sent.

### Integration tests (`apps/`)

Integration tests exercise the full app binary end-to-end. The test CMakeLists.txt builds
the real application, and each test:

1. Creates a PTY pair.
2. Spawns the app binary as a subprocess, pointing `--port` at the slave device path.
3. Acts as the **device side** on the master fd — reads the COBS-framed request the app
   sends and writes back a COBS-framed response.
4. Waits for the subprocess to exit, then asserts the exit code and the response file
   content.

#### CMake structure

The integration-test CMakeLists.txt lives alongside (or under) the app it tests. It
explicitly builds the app target so that a plain `cmake --build` / `ctest` invocation is
self-contained:

```cmake
# apps/linux_uart_cobs_client/tests/CMakeLists.txt

# Build the application under test as part of this target tree.
add_subdirectory(.. linux_uart_cobs_client_build)

add_executable(linux_uart_cobs_client_it
    src/it_linux_uart_cobs_client.cpp
)
target_link_libraries(linux_uart_cobs_client_it PRIVATE gtest gtest_main)

# Inject the path to the built binary at compile time so tests can exec() it.
target_compile_definitions(linux_uart_cobs_client_it PRIVATE
    APP_BINARY="$<TARGET_FILE:linux_uart_cobs_client>"
)

add_test(NAME linux_uart_cobs_client_it COMMAND linux_uart_cobs_client_it)
```

#### Test fixture

The fixture owns the PTY pair and the subprocess lifetime:

```cpp
#include <pty.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <stdexcept>

struct AppUnderTest {
    int  master_fd{-1};
    int  slave_fd{-1};
    pid_t pid{-1};
    std::string slave_path;

    // Open the PTY pair.  slave_fd is kept open so the PTY stays alive
    // even before the subprocess opens it.
    AppUnderTest() {
        char name[64]{};
        if (openpty(&master_fd, &slave_fd, name, nullptr, nullptr) != 0) {
            throw std::runtime_error("openpty failed");
        }
        slave_path = name;
    }

    // Spawn the application binary, handing it the slave path as --port.
    void launch(const std::vector<std::string>& extra_args) {
        pid = fork();
        if (pid < 0) { throw std::runtime_error("fork failed"); }
        if (pid == 0) {
            // Child: close the master fd — it belongs to the test process.
            close(master_fd);
            std::vector<const char*> argv = { APP_BINARY, "--port", slave_path.c_str() };
            for (const auto& a : extra_args) { argv.push_back(a.c_str()); }
            argv.push_back(nullptr);
            execv(APP_BINARY, const_cast<char* const*>(argv.data()));
            _exit(127); // execv failed
        }
        // Parent: close slave_fd — the child (serialib) now owns it.
        close(slave_fd);
        slave_fd = -1;
    }

    // Wait for the subprocess to exit and return its exit code.
    int wait() {
        int status{};
        waitpid(pid, &status, 0);
        pid = -1;
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }

    ~AppUnderTest() {
        if (pid  > 0)  { kill(pid, SIGTERM); wait(); }
        if (slave_fd  >= 0) { close(slave_fd);  }
        if (master_fd >= 0) { close(master_fd); }
    }
    AppUnderTest(const AppUnderTest&) = delete;
    AppUnderTest& operator=(const AppUnderTest&) = delete;
};
```

#### COBS framing on the master (device-simulator) side

The test must speak the same COBS protocol the app uses. The existing
`CobsFrameReader` and `CobsFrameWriter` classes can be reused in the test binary by
linking against `cobs_frame_reader` and `cobs_frame_writer` CMake targets. The test wraps
`master_fd` in thin `read`/`write` helpers that satisfy the `Reader` / `Writer` interfaces.

#### Interaction flow

```
Test process                        App subprocess
─────────────────────────────────   ──────────────────────────────────
Create PTY pair
Write request file (binary/JSON)
launch(--request=..., --response=..)  ─spawn──► openDevice(slave_path)
                                                 flushReceiver()
                                                 cobs_writer.write(request)
master_fd: read COBS frame ◄──────── slave: bytes sent
master_fd: write COBS response ─────► slave: bytes received
                                                 cobs_reader.read() → payload
                                                 write response file
                                                 closeDevice() → exit 0
int rc = wait()
ASSERT_EQ(rc, 0)
Read response file, assert content
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
