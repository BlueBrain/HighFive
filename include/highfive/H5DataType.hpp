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

#include <vector>

#include "H5Object.hpp"
#include "bits/H5Utils.hpp"

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
    bool isVariableStr() const;

    ///
    /// \brief Returns whether the type is a fixed-length string
    ///
    bool isFixedLenStr() const;

    ///
    /// \brief Check the DataType was default constructed.
    /// Such value might represent auto-detection of the datatype from a buffer
    ///
    bool empty() const noexcept;

    /// \brief Returns whether the type is a Reference
    bool isReference() const;

  protected:
    using Object::Object;

    friend class Attribute;
    friend class File;
    friend class DataSet;
};

template <size_t N>
using FixedLengthString = std::array<char, N>;

template <size_t N>
FixedLengthString<N> fromString(const char* str) {
    FixedLengthString<N> toStr;
    size_t i = 0;
    while (i < N && str[i] != '\0') {
        toStr[i] = str[i];
        ++i;
    }
    return toStr;
}

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
        member_def(std::string t_name, DataType t_base_type, size_t t_offset = 0)
            : name(std::move(t_name))
            , base_type(std::move(t_base_type))
            , offset(t_offset) {}
        std::string name;
        DataType base_type;
        size_t offset;
    };

    CompoundType(const CompoundType& other) = default;

    ///
    /// \brief Initializes a compound type from a vector of member definitions
    /// \param t_members
    /// \param size
    inline CompoundType(const std::vector<member_def>& t_members, size_t size = 0)
        : members(t_members) {
        create(size);
    }
    inline CompoundType(std::vector<member_def>&& t_members, size_t size = 0)
        : members(std::move(t_members)) {
        create(size);
    }
    inline CompoundType(const std::initializer_list<member_def>& t_members,
                        size_t size = 0)
        : members(t_members) {
        create(size);
    }

    /// \brief Commit datatype into the given Object
    /// \param object Location to commit object into
    /// \param name Name to give the datatype
    inline void commit(const Object& object, const std::string& name) const;

    /// \brief Get read access to the CompoundType members
    inline const std::vector<member_def>& getMembers() const noexcept {
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

///
/// \brief Create a enum HDF5 datatype
///
/// \code{.cpp}
/// enum class Position {
///     FIRST = 1,
///     SECOND = 2,
/// };
///
/// EnumType<Position> create_enum_position() {
///     return {{"FIRST", Position::FIRST},
///             {"SECOND", Position::SECOND}};
/// }
///
/// // You have to register the type inside HighFive
/// HIGHFIVE_REGISTER_TYPE(Position, create_enum_position)
///
/// void write_first(H5::File& file) {
///     auto dataset = file.createDataSet("/foo", Position::FIRST);
/// }
/// \endcode
template<typename T>
class EnumType: public DataType {
public:
    ///
    /// \brief Use for defining a member of enum type
    struct member_def {
        member_def(const std::string& t_name, T t_value)
            : name(t_name)
            , value(std::move(t_value)) {}
        std::string name;
        T value;
    };

    EnumType(const EnumType& other) = default;

    EnumType(const std::vector<member_def>& t_members)
        : members(t_members) {
        create();
    }

    EnumType(std::initializer_list<member_def> t_members)
        : EnumType(std::vector<member_def>({t_members})) {}

    /// \brief Commit datatype into the given Object
    /// \param object Location to commit object into
    /// \param name Name to give the datatype
    void commit(const Object& object, const std::string& name) const;

private:
    std::vector<member_def> members;

    void create();
};


/// \brief Create a DataType instance representing type T
template <typename T>
DataType create_datatype();


/// \brief Create a DataType instance representing type T and perform a sanity check on its size
template <typename T>
DataType create_and_check_datatype();


}  // namespace HighFive


/// \brief Macro to extend datatype of HighFive
///
/// This macro has to be called outside of any namespace.
///
/// \code{.cpp}
/// enum FooBar { FOO = 1, BAR = 2 };
/// EnumType create_enum_foobar() {
///    return EnumType<FooBar>({{"FOO", FooBar::FOO},
///                             {"BAR", FooBar::BAR}});
/// }
/// HIGHFIVE_REGISTER_TYPE(FooBar, create_enum_foobar)
/// \endcode
#define HIGHFIVE_REGISTER_TYPE(type, function) \
    namespace HighFive {                       \
    template<>                                 \
    DataType create_datatype<type>() {         \
        return function();                     \
    }                                          \
    }

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
