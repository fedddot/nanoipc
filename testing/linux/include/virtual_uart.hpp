#ifndef VIRTUAL_UART_HPP
#define VIRTUAL_UART_HPP

#include <chrono>
#include <fcntl.h>
#include <poll.h>
#include <pty.h>
#include <unistd.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace nanoipc_testing {
    /// @brief GoogleTest fixture that manages a virtual PTY pair for UART testing.
    ///
    /// Opens a master/slave PTY pair in SetUp(). Tests pass slave_path() to
    /// serialib::openDevice() (or as --port to an app under test) and use
    /// write_data() / read_data() / read_until_byte() to drive the master side.
    class VirtualUart : public ::testing::Test {
    public:
        /// @brief Returns the slave device path (e.g. /dev/pts/N).
        const std::string& slave_path() const { return m_slave_path; }

        /// @brief Writes raw bytes to the master side (simulates data arriving from the remote device).
        void write_data(const std::vector<std::uint8_t>& data) {
            const auto written = ::write(m_master_fd, data.data(), data.size());
            if (written < 0 || static_cast<std::size_t>(written) != data.size()) {
                throw std::runtime_error("write_data: write failed");
            }
        }

        /// @brief Reads exactly n bytes from the master side.
        std::vector<std::uint8_t> read_data(std::size_t n, int timeout_ms = 2000) {
            std::vector<std::uint8_t> buf(n);
            std::size_t total = 0;
            while (total < n) {
                struct pollfd pfd{m_master_fd, POLLIN, 0};
                if (::poll(&pfd, 1, timeout_ms) <= 0) {
                    throw std::runtime_error("read_data: timeout or poll error");
                }
                const auto got = ::read(m_master_fd, buf.data() + total, n - total);
                if (got <= 0) { throw std::runtime_error("read_data: read failed"); }
                total += static_cast<std::size_t>(got);
            }
            return buf;
        }

        /// @brief Reads raw bytes from the master until a delimiter byte is encountered (inclusive).
        ///
        /// This is a generic delimiter-based read utility for framing protocols that use byte delimiters
        /// (e.g., COBS frames ending with 0x00, or any similar delimiter-based protocol).
        /// The returned vector includes the delimiter byte at the end.
        ///
        /// @param delimiter_byte The byte value to read until
        /// @param timeout_ms Total timeout in milliseconds
        /// @param max_bytes Maximum bytes to accumulate before failing (prevents unbounded growth)
        /// @throws std::runtime_error if timeout expires, read fails, or max_bytes is exceeded
        std::vector<std::uint8_t> read_until_byte(std::uint8_t delimiter_byte, int timeout_ms = 2000, std::size_t max_bytes = 65536) {
            std::vector<std::uint8_t> result;
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);

            while (true) {
                const auto now = std::chrono::steady_clock::now();
                if (now >= deadline) {
                    throw std::runtime_error("read_until_byte: timeout");
                }

                const auto remaining_ms = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
                struct pollfd pfd{m_master_fd, POLLIN, 0};
                if (::poll(&pfd, 1, static_cast<int>(remaining_ms)) <= 0) {
                    throw std::runtime_error("read_until_byte: timeout or poll error");
                }

                std::uint8_t byte{};
                const auto got = ::read(m_master_fd, &byte, 1);
                if (got <= 0) {
                    throw std::runtime_error("read_until_byte: read failed");
                }

                result.push_back(byte);
                if (result.size() > max_bytes) {
                    throw std::runtime_error("read_until_byte: max_bytes exceeded");
                }

                if (byte == delimiter_byte) {
                    break;
                }
            }
            return result;
        }

    protected:
        void SetUp() override {
            char name[64]{};
            if (::openpty(&m_master_fd, &m_slave_fd, name, nullptr, nullptr) != 0) {
                throw std::runtime_error("SetUp: openpty failed");
            }
            m_slave_path = name;
            // Prevent inherited fds from leaking into forked child processes.
            ::fcntl(m_master_fd, F_SETFD, FD_CLOEXEC);
            ::fcntl(m_slave_fd, F_SETFD, FD_CLOEXEC);
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
}

#endif // VIRTUAL_UART_HPP
