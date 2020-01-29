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
    FixedLenStringArray(const std::string* iter_begin, const std::string* iter_end);

    explicit FixedLenStringArray(const std::vector<std::string> & vec);

    FixedLenStringArray(const std::initializer_list<std::string> &);

    ///
    /// \brief Append an std::string to buffer structure
    ///
    void push_back(const std::string&);

    ///
    /// \brief Retrieve a string from the structure as std::string
    ///
    std::string operator[](std::size_t index) const;


    // Container API
    inline std::size_t size() const noexcept { return datavec.size(); }
    inline char* data() const { return const_cast<char*>(datavec[0].data()); }
    void resize(std::size_t new_size) { datavec.resize(new_size); }

  private:
    typedef std::vector<std::array<char, N>> vector_t;
    vector_t datavec;
};


}  // namespace HighFive

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
