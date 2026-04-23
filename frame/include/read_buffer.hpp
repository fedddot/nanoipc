#ifndef	READ_BUFFER_HPP
#define	READ_BUFFER_HPP

#include <cstddef>
#include <cstdint>

namespace nanoipc {
	/// @brief Abstract interface for reading bytes from a buffer.
	///
	/// ReadBuffer defines the interface for accessing buffered byte data
	/// in a sequential manner. Implementations may be circular buffers,
	/// queues, or other data structures capable of storing bytes.
	class ReadBuffer {
	public:
		virtual ~ReadBuffer() noexcept = default;

		/// @brief Remove and return the oldest byte from the buffer.
		/// @return The oldest byte in the buffer, or 0 if the buffer is empty.
		virtual std::uint8_t pop_front() = 0;

		/// @brief Return the number of bytes currently in the buffer.
		/// @return The count of available bytes.
		virtual std::size_t size() const = 0;

		/// @brief Peek at a byte at the given index without removing it.
		/// @param index Zero-based offset from the front of the buffer.
		/// @return The byte at the given index.
		/// @throws std::out_of_range if index is >= size().
		virtual std::uint8_t get(const std::size_t index) const = 0;
	};
}

#endif // READ_BUFFER_HPP