/// @file example_client.cpp
/// @brief NanoIPC example client – runs on a PC (Linux / macOS).
///
/// Opens a serial port, sends an ExampleRequest to the Blue Pill server, and
/// prints the ExampleResponse result to stdout.
///
/// Usage:
///   example_client --port <dev> --baud <rate> --value <n> --action <add|subtract>
///
/// Example:
///   example_client --port /dev/ttyUSB0 --baud 9600 --value 5 --action add

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <chrono>
#include <thread>

// POSIX serial
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// NanoIPC
#include "nanoipc_ring_buffer.hpp"

// API
#include "example_api.hpp"
#include "example_request_writer.hpp"
#include "example_response_reader.hpp"

// =============================================================================
// Serial port helpers
// =============================================================================

static speed_t baud_to_speed(const uint32_t baud) {
    switch (baud) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:
            throw std::runtime_error("unsupported baud rate: " + std::to_string(baud));
    }
}

/// @brief Open a serial port and configure it for 8N1 raw I/O.
static int open_serial(const std::string& port, const uint32_t baud) {
    const int fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error("cannot open port " + port + ": " + std::strerror(errno));
    }

    termios tty{};
    if (::tcgetattr(fd, &tty) != 0) {
        ::close(fd);
        throw std::runtime_error("tcgetattr failed: " + std::string(std::strerror(errno)));
    }

    const speed_t speed = baud_to_speed(baud);
    ::cfsetispeed(&tty, speed);
    ::cfsetospeed(&tty, speed);

    // 8N1 raw mode
    ::cfmakeraw(&tty);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CRTSCTS;

    // Non-blocking reads
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;

    if (::tcsetattr(fd, TCSANOW, &tty) != 0) {
        ::close(fd);
        throw std::runtime_error("tcsetattr failed: " + std::string(std::strerror(errno)));
    }

    ::tcflush(fd, TCIOFLUSH);
    return fd;
}

// =============================================================================
// CLI parsing
// =============================================================================

struct Config {
    std::string port  = "/dev/ttyUSB0";
    uint32_t    baud  = 9600;
    int32_t     value = 0;
    api::Action action = api::Action::ADD;
};

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --port <dev> --baud <rate> --value <n> --action <add|subtract>\n";
}

static Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--port") && i + 1 < argc) {
            cfg.port = argv[++i];
        } else if ((arg == "--baud") && i + 1 < argc) {
            cfg.baud = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if ((arg == "--value") && i + 1 < argc) {
            cfg.value = static_cast<int32_t>(std::stol(argv[++i]));
        } else if ((arg == "--action") && i + 1 < argc) {
            const std::string act = argv[++i];
            if (act == "add") {
                cfg.action = api::Action::ADD;
            } else if (act == "subtract") {
                cfg.action = api::Action::SUBTRACT;
            } else {
                throw std::runtime_error("unknown action '" + act + "' (use add or subtract)");
            }
        } else {
            throw std::runtime_error("unknown argument: " + arg);
        }
    }
    return cfg;
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char* argv[]) {
    Config cfg;
    try {
        cfg = parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int fd = -1;
    try {
        fd = open_serial(cfg.port, cfg.baud);

        nanoipc::NanoipcRingBuffer<256> read_buffer;

        api::ExampleRequestWriter writer([fd](const std::uint8_t* data, const std::size_t size) {
            std::size_t written = 0;
            while (written < size) {
                const ssize_t n = ::write(fd, data + written, size - written);
                if (n < 0) {
                    throw std::runtime_error("write error: " + std::string(std::strerror(errno)));
                }
                written += static_cast<std::size_t>(n);
            }
        });

        api::ExampleResponseReader reader(&read_buffer);

        // Send the request
        writer.write(api::ExampleRequest{ cfg.value, cfg.action });

        // Poll for a response (timeout: 5 s)
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline) {
            // Drain any available bytes from the serial port into the ring buffer
            std::uint8_t tmp[64];
            const ssize_t n = ::read(fd, tmp, sizeof(tmp));
            if (n > 0) {
                for (ssize_t i = 0; i < n; ++i) {
                    read_buffer.push_back(tmp[i]);
                }
            }

            const auto response = reader.read();
            if (response.has_value()) {
                std::cout << response->result << "\n";
                ::close(fd);
                return EXIT_SUCCESS;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cerr << "error: timed out waiting for response\n";
        ::close(fd);
        return EXIT_FAILURE;

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        if (fd >= 0) ::close(fd);
        return EXIT_FAILURE;
    }
}
