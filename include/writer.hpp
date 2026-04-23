#ifndef	WRITER_HPP
#define	WRITER_HPP

namespace nanoipc {
	template <typename T>
	class Writer {
	public:
		virtual ~Writer() noexcept = default;
		virtual void write(const T& data) const = 0;
	};
}

#endif // WRITER_HPP
