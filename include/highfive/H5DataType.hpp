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

    // Store the list of currently added members (name, hid, offset)
    std::vector<std::tuple<const std::string&, hid_t, size_t>> member_list;
};
}

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
