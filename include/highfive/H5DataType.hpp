/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <type_traits>
#include <vector>

#include <H5Tpublic.h>

#include "H5Object.hpp"
#include "bits/H5Utils.hpp"

#include "bits/string_padding.hpp"
#include "H5PropertyList.hpp"

#include "bits/h5_wrapper.hpp"
#include "bits/h5t_wrapper.hpp"

namespace HighFive {


///
/// \brief Enum of Fundamental data classes
///
enum class DataTypeClass {
    Time = 1 << 1,
    Integer = 1 << 2,
    Float = 1 << 3,
    String = 1 << 4,
    BitField = 1 << 5,
    Opaque = 1 << 6,
    Compound = 1 << 7,
    Reference = 1 << 8,
    Enum = 1 << 9,
    VarLen = 1 << 10,
    Array = 1 << 11,
    Invalid = 0
};

inline DataTypeClass operator|(DataTypeClass lhs, DataTypeClass rhs) {
    using T = std::underlying_type<DataTypeClass>::type;
    return static_cast<DataTypeClass>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline DataTypeClass operator&(DataTypeClass lhs, DataTypeClass rhs) {
    using T = std::underlying_type<DataTypeClass>::type;
    return static_cast<DataTypeClass>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

class StringType;

///
/// \brief HDF5 Data Type
///
class DataType: public Object {
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
    /// given that it refers to the size of the control structure. For info see
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
    /// \brief Returns this datatype as a `StringType`.
    ///
    StringType asStringType() const;

    ///
    /// \brief Check the DataType was default constructed.
    ///
    bool empty() const noexcept;

    /// \brief Returns whether the type is a Reference
    bool isReference() const;

    /// \brief Get the list of properties for creation of this DataType
    DataTypeCreateProps getCreatePropertyList() const {
        return details::get_plist<DataTypeCreateProps>(*this, H5Tget_create_plist);
    }

  protected:
    using Object::Object;

    friend class Attribute;
    friend class File;
    friend class DataSet;
    friend class CompoundType;
    template <typename Derivate>
    friend class NodeTraits;
};


enum class CharacterSet : std::underlying_type<H5T_cset_t>::type {
    Ascii = H5T_CSET_ASCII,
    Utf8 = H5T_CSET_UTF8,
};

class StringType: public DataType {
  public:
    ///
    /// \brief For stings return the character set.
    ///
    CharacterSet getCharacterSet() const;

    ///
    /// \brief For fixed length stings return the padding.
    ///
    StringPadding getPadding() const;

  protected:
    using DataType::DataType;
    friend class DataType;
};

class FixedLengthStringType: public StringType {
  public:
    ///
    /// \brief Create a fixed length string datatype.
    ///
    /// The string will be `size` bytes long, regardless whether it's ASCII or
    /// UTF8. In particular, a string with `n` UFT8 characters in general
    /// requires `4*n` bytes.
    ///
    /// The string padding is subtle, essentially it's just a hint. While
    /// commonly, a null-terminated string is guaranteed to have one `'\0'`
    /// which marks the semantic end of the string, this is not enforced by
    /// HDF5. In fact, there are HDF5 files that contain strings that claim to
    /// be null-terminated but aren't.  The length of the buffer must be at
    /// least `size` bytes regardless of the padding. HDF5 will read or write
    /// `size` bytes, irrespective of when (if at all) the `\0` occurs.
    ///
    /// Note that when writing, passing `StringPadding::NullTerminated` is a
    /// guarantee to the reader that it contains a `\0`. Therefore, make sure
    /// that the string really is null-terminated. Otherwise prefer a
    /// null-padded string. This mearly states that the buffer is filled up
    /// with 0 or more `\0`.
    FixedLengthStringType(size_t size,
                          StringPadding padding,
                          CharacterSet character_set = CharacterSet::Ascii);
};

class VariableLengthStringType: public StringType {
  public:
    ///
    /// \brief Create a variable length string HDF5 datatype.
    ///
    VariableLengthStringType(CharacterSet character_set = CharacterSet::Ascii);
};


///
/// \brief create an HDF5 DataType from a C++ type
///
///  Support only basic data type
///
template <typename T>
class AtomicType: public DataType {
  public:
    AtomicType();

    using basic_type = T;
};


///
/// \brief Create a compound HDF5 datatype
///
class CompoundType: public DataType {
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
    inline CompoundType(const std::initializer_list<member_def>& t_members, size_t size = 0)
        : members(t_members) {
        create(size);
    }

    ///
    /// \brief Initializes a compound type from a DataType
    /// \param type
    inline CompoundType(DataType&& type)
        : DataType(type) {
        if (getClass() != DataTypeClass::Compound) {
            std::ostringstream ss;
            ss << "hid " << _hid << " does not refer to a compound data type";
            throw DataTypeException(ss.str());
        }
        size_t n_members = static_cast<size_t>(detail::h5t_get_nmembers(_hid));
        members.reserve(n_members);
        for (unsigned i = 0; i < n_members; i++) {
            char* name = detail::h5t_get_member_name(_hid, i);
            size_t offset = detail::h5t_get_member_offset(_hid, i);
            hid_t member_hid = detail::h5t_get_member_type(_hid, i);
            DataType member_type{member_hid};
            members.emplace_back(std::string(name), member_type, offset);

            detail::h5_free_memory(name);
        }
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
template <typename T>
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
        static_assert(std::is_enum<T>::value, "EnumType<T>::create takes only enum");
        if (members.empty()) {
            HDF5ErrMapper::ToException<DataTypeException>(
                "Could not create an enum without members");
        }
        create();
    }

    EnumType(std::initializer_list<member_def> t_members)
        : EnumType(std::vector<member_def>(t_members)) {}

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
/// namespace app {
/// enum FooBar { FOO = 1, BAR = 2 };
/// EnumType create_enum_foobar() {
///    return EnumType<FooBar>({{"FOO", FooBar::FOO},
///                             {"BAR", FooBar::BAR}});
/// }
/// }
///
/// HIGHFIVE_REGISTER_TYPE(FooBar, ::app::create_enum_foobar)
/// \endcode
#define HIGHFIVE_REGISTER_TYPE(type, function)                    \
    template <>                                                   \
    inline HighFive::DataType HighFive::create_datatype<type>() { \
        return function();                                        \
    }

#include "bits/H5DataType_misc.hpp"
