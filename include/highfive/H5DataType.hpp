/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5DATATYPE_HPP
#define H5DATATYPE_HPP

#include "H5Object.hpp"

namespace HighFive {


///
/// \brief Enum of Fundamental data classes
///
enum class DataTypeClass {
    Time,
    Integer,
    Float,
    String,
    BitField,
    Opaque,
    Compound,
    Reference,
    Enum,
    VarLen,
    Array,
    Invalid
};


///
/// \brief HDF5 Data Type
///
class DataType : public Object {
  public:
    DataType();

    bool operator==(const DataType& other) const;

    bool operator!=(const DataType& other) const;

    ///
    /// \brief Return the fundamental type.
    ///
    DataTypeClass getClass() const;

    ///
    /// \brief Returns the length (in bytes) of this type elements
    ///
    /// Notice that the size of variable length sequences may have limited applicability
    ///   given that it refers to the size of the control structure. For info see
    ///   https://support.hdfgroup.org/HDF5/doc/RM/RM_H5T.html#Datatype-GetSize
    size_t getSize() const;

    ///
    /// \brief Returns a friendly description of the type (e.g. Float32)
    ///
    std::string string() const;

    ///
    /// \brief Returns whether the type is a variable-length string
    ///
    bool isVariableString() const;

    ///
    /// \brief Returns whether the type is a fixed-length string
    ///
    bool isFixedLengthString() const;

  protected:
    friend class Attribute;
    friend class File;
    friend class DataSet;

    DataType(hid_t type_hid);
};

///
/// \brief create an HDF5 DataType from a C++ type
///
///  Support only basic data type
///
template <typename T>
class AtomicType : public DataType {
  public:
    AtomicType();

    typedef T basic_type;
};


///
/// \brief A structure representing a set of fixed-length strings
///
/// Although fixed-len arrays can be created 'raw' without the need for
/// this structure, to retrieve results efficiently it must be used.
///
template <std::size_t N>
class FixedLenStringArray {
  public:
    FixedLenStringArray() = default;

    ///
    /// \brief Create a FixedStringArray from a raw contiguous buffer
    ///
    FixedLenStringArray(const char array[][N], std::size_t length);

    ///
    /// \brief Create a FixedStringArray from a sequence of strings.
    ///
    /// Such conversion involves a copy, original vector is not modified
    ///
    explicit FixedLenStringArray(const std::vector<std::string> & vec);

    FixedLenStringArray(const std::string* iter_begin, const std::string* iter_end);

    FixedLenStringArray(const std::initializer_list<std::string> &);

    ///
    /// \brief Append an std::string to the buffer structure
    ///
    void push_back(const std::string&);

    void push_back(const std::array<char, N>&);

    ///
    /// \brief Retrieve a string from the structure as std::string
    ///
    std::string getString(std::size_t index) const;

    // Container interface
    inline const char* operator[](std::size_t i) const noexcept {
        return datavec[i].data();
    }
    inline const char* at(std::size_t i) const {
        return datavec.at(i).data();
    }
    inline bool empty() const noexcept {
        return datavec.empty();
    }
    inline std::size_t size() const noexcept {
        return datavec.size();
    }
    inline void resize(std::size_t n) {
        datavec.resize(n);
    }
    inline const char* front() const {
        return datavec.front().data();
    }
    inline const char* back() const {
        return datavec.back().data();
    }
    inline char* data() noexcept {
        return datavec[0].data();
    }
    inline const char* data() const noexcept {
        return datavec[0].data();
    }

  private:
    using vector_t = typename std::vector<std::array<char, N>>;

  public:
    // Use the underlying iterator
    using iterator = typename vector_t::iterator;
    using const_iterator = typename vector_t::const_iterator;
    using reverse_iterator = typename vector_t::reverse_iterator;
    using const_reverse_iterator = typename vector_t::const_reverse_iterator;
    using value_type = typename vector_t::value_type;

    inline iterator begin() noexcept {
        return datavec.begin();
    }
    inline iterator end() noexcept {
        return datavec.end();
    }
    inline const_iterator begin() const noexcept {
        return datavec.begin();
    }
    inline const_iterator cbegin() const noexcept {
        return datavec.cbegin();
    }
    inline const_iterator end() const noexcept {
        return datavec.end();
    }
    inline const_iterator cend() const noexcept {
        return datavec.cend();
    }
    inline reverse_iterator rbegin() noexcept {
        return datavec.rbegin();
    }
    inline reverse_iterator rend() noexcept {
        return datavec.rend();
    }
    inline const_reverse_iterator rbegin() const noexcept {
        return datavec.rbegin();
    }
    inline const_reverse_iterator rend() const noexcept {
        return datavec.rend();
    }

  private:
    vector_t datavec;
};

}  // namespace HighFive

#include "bits/H5DataType_misc.hpp"

#endif  // H5DATATYPE_HPP
