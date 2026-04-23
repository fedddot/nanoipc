#ifndef	READER_HPP
#define	READER_HPP

#include <optional>

namespace nanoipc {
	/// @brief Generic interface for reading typed data.
	///
	/// Reader is a template-based interface that defines a contract for
	/// reading objects of type T. Implementations may deserialize, transform,
	/// or otherwise produce objects from various sources.
	///
	/// @tparam T The type of data being read.
	template <typename T>
	class Reader {
	public:
		virtual ~Reader() noexcept = default;

		/// @brief Read and return the next object of type T.
		/// @return An optional containing the read object, or std::nullopt if no object is available.
		virtual std::optional<T> read() = 0;
	};
}

#endif // READER_HPP