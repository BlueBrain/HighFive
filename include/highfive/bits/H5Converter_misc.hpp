/*
 *  Copyright (c) 2022 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <type_traits>

#include "H5Inspector_misc.hpp"
#include "../H5DataType.hpp"

namespace HighFive {
namespace details {

template <class T>
struct is_std_string {
    static constexpr bool value =
        std::is_same<typename inspector<T>::base_type, std::string>::value;
};

template <class T, class V = void>
struct enable_shallow_copy
    : public std::enable_if<!is_std_string<T>::value && inspector<T>::is_trivially_copyable, V> {};

template <class T, class V = void>
struct enable_deep_copy
    : public std::enable_if<!is_std_string<T>::value && !inspector<T>::is_trivially_copyable, V> {};

template <class T, class V = void>
struct enable_string_copy: public std::enable_if<is_std_string<T>::value, V> {};


template <typename T, bool IsReadOnly>
struct ShallowCopyBuffer {
    using type = unqualified_t<T>;
    using hdf5_type =
        typename std::conditional<IsReadOnly,
                                  typename std::add_const<typename inspector<T>::hdf5_type>::type,
                                  typename inspector<T>::hdf5_type>::type;

    ShallowCopyBuffer() = delete;

    explicit ShallowCopyBuffer(typename std::conditional<IsReadOnly, const T&, T&>::type val)
        : ptr(inspector<T>::data(val)) {};

    hdf5_type* getPointer() const {
        return ptr;
    }

    hdf5_type* begin() const {
        return getPointer();
    }

    void unserialize(T& /* val */) const {
        /* nothing to do. */
    }

  private:
    hdf5_type* ptr;
};

template <class T>
struct DeepCopyBuffer {
    using type = unqualified_t<T>;
    using hdf5_type = typename inspector<type>::hdf5_type;

    explicit DeepCopyBuffer(const std::vector<size_t>& _dims)
        : buffer(compute_total_size(_dims))
        , dims(_dims) {}

    hdf5_type* getPointer() {
        return buffer.data();
    }

    hdf5_type const* getPointer() const {
        return buffer.data();
    }

    hdf5_type* begin() {
        return getPointer();
    }

    hdf5_type const* begin() const {
        return getPointer();
    }

    void unserialize(T& val) const {
        inspector<type>::unserialize(buffer.data(), dims, val);
    }

  private:
    std::vector<hdf5_type> buffer;
    std::vector<size_t> dims;
};

enum class BufferMode { Read, Write };


///
/// \brief String length in bytes excluding the `\0`.
///
inline size_t char_buffer_length(char const* const str, size_t max_string_size) {
    for (size_t i = 0; i < max_string_size; ++i) {
        if (str[i] == '\0') {
            return i;
        }
    }

    return max_string_size;
}


///
/// \brief A buffer for reading/writing strings.
///
/// A string in HDF5 can be represented as a fixed or variable length string.
/// The important difference for this buffer is that `H5D{read,write}` expects
/// different input depending on whether the strings are fixed or variable length.
/// For fixed length strings, it expects an array of chars, i.e. one string
/// packed after the other contiguously. While for variable length strings it
/// expects a list of pointers to the beginning of each string. Variable length
/// string must be null-terminated; because that's how their length is
/// determined.
///
/// This buffer hides the difference between fixed and variable length strings
/// by having internal data structures available for both cases at compile time.
/// The choice which internal buffer to use is made at runtime.
///
/// Consider an HDF5 dataset with N fixed-length strings, each of which is M
/// characters long. Then the in-memory strings are copied into an internal
/// buffer of size N*M. If null- or space-padded the buffer should be filled
/// with the appropriate character. This is important if the in-memory strings
/// are less than M characters long.
///
/// An HDF5 dataset with N variable-length strings (all null-terminated) uses
/// the internal list of pointers to the beginning of each string. Those
/// pointers can either point to the in-memory strings themselves, if those
/// strings are known to be null-terminated. Otherwise the in-memory strings are
/// copied to an internal buffer of null-terminated strings; and the pointer
/// points to the start of the string in the internal buffer.
///
/// This class is responsible for arranging the strings properly before passing
/// the buffers to HDF5. To keep this class generic, it provides a generic
/// read/write interface to the internal strings, i.e. a pointer with a size.
/// For reading from the buffer the proxy is called `StringConstView`. This
/// proxy object is to be used by the `inspector` to copy from the buffer into
/// the final destination, e.g. an `std::string`.  Similarly, there's a proxy
/// object for serializing into the buffer, i.e. the `StringView`. Again the
/// `inspector` is responsible for obtaining the pointer, size and padding of
/// the string.
///
/// Nomenclature:
///   - size of a string is the number of bytes required to store the string,
///     including the null character for null-terminated strings.
///
///   - length of a string is the number of bytes without the null character.
///
/// Note: both 'length' and 'size' are counted in number of bytes, not number
///   of symbols or characters. Even for UTF8 strings.
template <typename T, BufferMode buffer_mode>
struct StringBuffer {
    using type = unqualified_t<T>;
    using hdf5_type = typename inspector<type>::hdf5_type;

    class StringView {
      public:
        StringView(StringBuffer<T, buffer_mode>& _buffer, size_t _i)
            : buffer(_buffer)
            , i(_i) {}

        ///
        /// \brief Assign the in-memory string to the buffer.
        ///
        /// This method copies the in-memory string to the appropriate
        /// internal buffer as needed.
        ///
        /// The `length` is the length of the string in bytes.
        void assign(char const* data, size_t length, StringPadding pad) {
            if (buffer.isVariableLengthString()) {
                if (pad == StringPadding::NullTerminated) {
                    buffer.variable_length_pointers[i] = data;
                } else {
                    buffer.variable_length_buffer[i] = std::string(data, length);
                    buffer.variable_length_pointers[i] = buffer.variable_length_buffer[i].data();
                }
            } else if (buffer.isFixedLengthString()) {
                // If the buffer is fixed-length and null-terminated, then
                // `buffer.string_length` doesn't include the null-character.
                if (length > buffer.string_max_length) {
                    throw std::invalid_argument("String length too big.");
                }

                memcpy(&buffer.fixed_length_buffer[i * buffer.string_size], data, length);
            }
        }

      private:
        StringBuffer<T, buffer_mode>& buffer;
        size_t i;
    };


    class StringConstView {
      public:
        StringConstView(const StringBuffer<T, buffer_mode>& _buffer, size_t _i)
            : buffer(_buffer)
            , i(_i) {}

        /// \brief Pointer to the first byte of the string.
        ///
        /// The valid indices for this pointer are: 0, ..., length() - 1.
        char const* data() const {
            if (buffer.isVariableLengthString()) {
                return buffer.variable_length_pointers[i];
            } else {
                return &buffer.fixed_length_buffer[i * buffer.string_size];
            }
        }

        /// \brief Length of the string in bytes.
        ///
        /// Note that for null-terminated strings the "length" doesn't include
        /// the null character. Hence, if storing this string as a
        /// null-terminated string, the destination buffer needs to be at least
        /// `length() + 1` bytes long.
        size_t length() const {
            if (buffer.isNullTerminated()) {
                return char_buffer_length(data(), buffer.string_size);
            } else {
                return buffer.string_max_length;
            }
        }

      private:
        const StringBuffer<T, buffer_mode>& buffer;
        size_t i;
    };


    class Iterator {
      public:
        Iterator(StringBuffer<T, buffer_mode>& _buffer, size_t _pos)
            : buffer(_buffer)
            , pos(_pos) {}

        Iterator operator+(size_t n_strings) const {
            return Iterator(buffer, pos + n_strings);
        }

        void operator+=(size_t n_strings) {
            pos += n_strings;
        }

        StringView operator*() {
            return StringView(buffer, pos);
        }

        StringConstView operator*() const {
            return StringConstView(buffer, pos);
        }

      private:
        StringBuffer<T, buffer_mode>& buffer;
        size_t pos;
    };

    StringBuffer(std::vector<size_t> _dims, const DataType& _file_datatype)
        : file_datatype(_file_datatype.asStringType())
        , padding(file_datatype.getPadding())
        , string_size(file_datatype.isVariableStr() ? size_t(-1) : file_datatype.getSize())
        , string_max_length(string_size - size_t(isNullTerminated()))
        , dims(_dims) {
        if (string_size == 0 && isNullTerminated()) {
            throw DataTypeException(
                "Fixed-length, null-terminated need at least one byte to store the "
                "null-character.");
        }

        auto n_strings = compute_total_size(dims);
        if (isVariableLengthString()) {
            variable_length_buffer.resize(n_strings);
            variable_length_pointers.resize(n_strings);
        } else {
            char pad = padding == StringPadding::SpacePadded ? ' ' : '\0';
            fixed_length_buffer.assign(n_strings * string_size, pad);
        }
    }

    bool isVariableLengthString() const {
        return file_datatype.isVariableStr();
    }

    bool isFixedLengthString() const {
        return file_datatype.isFixedLenStr();
    }

    bool isNullTerminated() const {
        return file_datatype.getPadding() == StringPadding::NullTerminated;
    }


    void* getPointer() {
        if (file_datatype.isVariableStr()) {
            return variable_length_pointers.data();
        } else {
            return fixed_length_buffer.data();
        }
    }

    Iterator begin() {
        return Iterator(*this, 0ul);
    }

    void unserialize(T& val) {
        inspector<type>::unserialize(begin(), dims, val);
    }

  private:
    StringType file_datatype;
    StringPadding padding;
    // Size of buffer required to store the string.
    // Meaningful for fixed length strings only.
    size_t string_size;
    // Maximum length of string.
    size_t string_max_length;
    std::vector<size_t> dims;

    std::vector<char> fixed_length_buffer;
    std::vector<std::string> variable_length_buffer;
    std::vector<
        typename std::conditional<buffer_mode == BufferMode::Write, const char, char>::type*>
        variable_length_pointers;
};


template <typename T, typename Enable = void>
struct Writer;

template <typename T>
struct Writer<T, typename enable_shallow_copy<T>::type>: public ShallowCopyBuffer<T, true> {
  private:
    using super = ShallowCopyBuffer<T, true>;

  public:
    explicit Writer(const T& val,
                    const std::vector<size_t>& /* dims */,
                    const DataType& /* file_datatype */)
        : super(val) {};
};

template <typename T>
struct Writer<T, typename enable_deep_copy<T>::type>: public DeepCopyBuffer<T> {
    explicit Writer(const T& val,
                    const std::vector<size_t>& _dims,
                    const DataType& /* file_datatype */)
        : DeepCopyBuffer<T>(_dims) {
        inspector<T>::serialize(val, _dims, this->begin());
    }
};

template <typename T>
struct Writer<T, typename enable_string_copy<T>::type>: public StringBuffer<T, BufferMode::Write> {
    explicit Writer(const T& val, const std::vector<size_t>& _dims, const DataType& _file_datatype)
        : StringBuffer<T, BufferMode::Write>(_dims, _file_datatype) {
        inspector<T>::serialize(val, _dims, this->begin());
    }
};

template <typename T, typename Enable = void>
struct Reader;

template <typename T>
struct Reader<T, typename enable_shallow_copy<T>::type>: public ShallowCopyBuffer<T, false> {
  private:
    using super = ShallowCopyBuffer<T, false>;
    using type = typename super::type;

  public:
    Reader(const std::vector<size_t>&, type& val, const DataType& /* file_datatype */)
        : super(val) {}
};

template <typename T>
struct Reader<T, typename enable_deep_copy<T>::type>: public DeepCopyBuffer<T> {
  private:
    using super = DeepCopyBuffer<T>;
    using type = typename super::type;

  public:
    Reader(const std::vector<size_t>& _dims, type&, const DataType& /* file_datatype */)
        : super(_dims) {}
};


template <typename T>
struct Reader<T, typename enable_string_copy<T>::type>: public StringBuffer<T, BufferMode::Write> {
  public:
    explicit Reader(const std::vector<size_t>& _dims,
                    const T& /* val */,
                    const DataType& _file_datatype)
        : StringBuffer<T, BufferMode::Write>(_dims, _file_datatype) {}
};

struct data_converter {
    template <typename T>
    static Writer<T> serialize(const typename inspector<T>::type& val,
                               const std::vector<size_t>& dims,
                               const DataType& file_datatype) {
        return Writer<T>(val, dims, file_datatype);
    }

    template <typename T>
    static Reader<T> get_reader(const std::vector<size_t>& dims,
                                T& val,
                                const DataType& file_datatype) {
        inspector<T>::prepare(val, dims);
        return Reader<T>(dims, val, file_datatype);
    }
};

}  // namespace details
}  // namespace HighFive
