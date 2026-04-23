#ifndef	WRITER_HPP
#define	WRITER_HPP

namespace nanoipc {
	/// @brief Generic interface for writing typed data.
	///
	/// Writer is a template-based interface that defines a contract for
	/// writing objects of type T. Implementations may serialize, transform,
	/// or otherwise process objects to various destinations.
	///
	/// @tparam T The type of data being written.
	template <typename T>
	class Writer {
	public:
		virtual ~Writer() noexcept = default;

		/// @brief Write the given object of type T.
		/// @param data The data to write.
		virtual void write(const T& data) const = 0;
	};
}

#endif // WRITER_HPP
