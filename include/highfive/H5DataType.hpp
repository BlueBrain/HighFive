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



namespace HighFive{

struct TypeMapper;

///
/// \brief HDF5 Data Type
///
class DataType : public Object{
public:
    DataType();

    bool operator==(const DataType & other) const;
    bool operator!=(const DataType & other) const;

    size_t getSize() const;
    void   setSize(size_t length) const;
    hid_t  getClass() const;
    bool   isVarLength() const;

protected:

    friend class Attribute;
    friend class File;
    friend class DataSet;
    friend class StringType;
};


///
/// \brief create an HDF5 DataType from a C++ type
///
///  Support only basic data type
///
template<typename T>
class AtomicType : public DataType{
public:

    AtomicType();

    typedef T basic_type;
};



class StringType : public AtomicType<std::string> {
public:
    enum PaddingType {
        NULLTERM = H5T_STR_NULLTERM,
        NULLPAD  = H5T_STR_NULLPAD,
        SPACEPAD = H5T_STR_SPACEPAD
    };

    StringType(const DataType & other);
    ~StringType();

    // Defaults to Variable-length, Null-Terminated string
    StringType(size_t fixedLengthTo=H5T_VARIABLE);

    void setFixedLengthTo(size_t fixedLengthTo);
    void setStrPad(PaddingType pad_t);
    PaddingType getStrPad();

    char* prepareRead(std::string & str);
    void postRead(std::string & str);

    char* prepareWrite(std::string & str);
    void postWrite(std::string & str);

private:
    // We keep the pointer passed to the read f
    char * _buf=NULL;
};


}

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
