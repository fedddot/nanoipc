#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "read_buffer.hpp"

namespace nanoipc {
    /// @brief Fixed-capacity ring buffer implementing the ReadBuffer interface.
    ///
    /// RingBuffer provides a compact, header-only circular buffer that
    /// implements nanoipc::ReadBuffer so it can be used with NanoIpcReader. The
    /// buffer supports writing bytes with push_back() and reading them via
    /// pop_front(), size(), and get().
    ///
    /// Template parameter N specifies the capacity (maximum number of bytes the
    /// buffer can hold).
    ///
    /// Notes:
    /// - This implementation does not perform any synchronization. If used from
    ///   multiple threads or from interrupt context, the caller must provide
    ///   appropriate synchronization or use an ISR-safe variant.
    /// - When the buffer is full, push_back() overwrites the oldest byte.
    template <std::size_t N>
    class RingBuffer : public ReadBuffer {
    public:
        /// @brief Construct an empty ring buffer.
        RingBuffer() : m_head(0), m_tail(0), m_size(0) {}

        /// @brief Append a byte to the back of the buffer.
        ///
        /// If the buffer is full the oldest byte is overwritten.
        void push_back(std::uint8_t v) {
            if (m_size < N) {
                m_data[m_tail] = v;
                m_tail = (m_tail + 1) % N;
                ++m_size;
            } else {
                // overwrite oldest
                m_data[m_tail] = v;
                m_tail = (m_tail + 1) % N;
                m_head = (m_head + 1) % N;
            }
        }

        /// @brief Remove and return the oldest byte from the buffer.
        /// @note Returns 0 if the buffer is empty.
        std::uint8_t pop_front() override {
            if (m_size == 0) return 0;
            const std::uint8_t v = m_data[m_head];
            m_head = (m_head + 1) % N;
            --m_size;
            return v;
        }

        /// @brief Number of bytes currently in the buffer.
        std::size_t size() const override {
            return m_size;
        }

        /// @brief Peek at a byte without removing it.
        /// @param index Zero-based offset from the front of the buffer.
        std::uint8_t get(const std::size_t index) const override {
            return m_data[(m_head + index) % N];
        }

    private:
        std::array<std::uint8_t, N> m_data;
        std::size_t m_head;
        std::size_t m_tail;
        std::size_t m_size;
    };
}

#endif // RING_BUFFER_HPP
