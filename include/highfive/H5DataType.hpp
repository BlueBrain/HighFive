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

struct TypeMapper;

///
/// \brief HDF5 Data Type
///
class DataType : public Object {
  public:
    DataType();

    bool operator==(const DataType& other) const;

    bool operator!=(const DataType& other) const;

    ///
    /// \brief Get the size of the datatype in bytes
    ///
    /// The datatype must already have been created or an exception will be
    /// thrown
    size_t getSize() const;

    ///
    /// \brief Test if the type is a variable length string
    ///
    /// The datatype must already have been created or an exception will be
    /// thrown
    bool isVariableStr() const;

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
    /// \brief Add a new member into the data type
    /// \param name Name of the member
    /// \param base_type HDF5 registered data type of the member
    /// \param offset Position of this member inside the type (optional when using autoCreate)
    void addMember(const std::string& name, hid_t base_type, size_t padding);

    ///
    /// \brief Add a new member into the data type
    /// \param name Name of the member
    /// \param base_type DataType of the member
    /// \param offset Position of this member inside the type (optional when using autoCreate)
    void addMember(const std::string& name, HighFive::DataType base_type, size_t padding);

    ///
    /// \brief Automatically create the type from the set of members
    ///        using standard struct alignment.
    void autoCreate();

    ///
    /// \brief Manually create the type from the members (including their offsets).
    /// \param total_size Total size of the datatype in bytes.
    void manualCreate(size_t total_size);

    ///
    /// \brief Commit datatype into the given Object
    /// \param object Location to commit object into
    /// \param name Name to give the datatype
    void commit(const Object& object, const std::string& name);



private:

    struct member_def {
        std::string name;
        hid_t base_type;
        size_t offset;
    };

    // Store the list of currently added members (name, hid, offset)
    std::vector<member_def> member_list;
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
