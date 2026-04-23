#ifndef	READ_BUFFER_HPP
#define	READ_BUFFER_HPP

#include <cstddef>
#include <cstdint>

namespace nanoipc {
	class ReadBuffer {
	public:
		virtual ~ReadBuffer() noexcept = default;
		virtual std::uint8_t pop_front() = 0;
		virtual std::size_t size() const = 0;
		virtual std::uint8_t get(const std::size_t index) const = 0;
	};
}

#endif // READ_BUFFER_HPP