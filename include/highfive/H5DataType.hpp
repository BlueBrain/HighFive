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
    /// \brief Test if the type is a variable length string
    ///
    /// The datatype must already have been created or an exception will be
    /// thrown
    bool isVariableStr() const;

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

  protected:
    friend class Attribute;
    friend class File;
    friend class DataSet;
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
/// \brief Create a compound HDF5 datatype
///
class CompoundType : public DataType {
public:
    ///
    /// \brief Use for defining a sub-type of compound type
    struct member_def {
        std::string name;
        DataType base_type;
        size_t offset = 0;
    };

    CompoundType(const CompoundType& other) = default;

    CompoundType(std::vector<member_def> t_members, size_t size = 0)
      : members(std::move(t_members)) {
        create(size);
    }

    CompoundType(std::initializer_list<member_def> t_members, size_t size = 0)
        : CompoundType(std::vector<member_def>({t_members}), size) {}

    /// \brief Commit datatype into the given Object
    /// \param object Location to commit object into
    /// \param name Name to give the datatype
    void commit(const Object& object, const std::string& name) const;

    /// \brief Get read access to the CompoundType members
    const std::vector<member_def>& get_members() const {
        return members;
    }

private:

    /// A vector of the member_def members of this CompoundType
    std::vector<member_def> members;

    /// \brief Automatically create the type from the set of members
    ///        using standard struct alignment.
    /// \param size Total size of data type
    void create(size_t size = 0);
};


/// \brief Create a DataType instance representing type T
template <typename T>
DataType create_datatype();


/// \brief Create a DataType instance representing type T and perform a sanity check on its size
template <typename T>
DataType create_and_check_datatype();


}

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
