#ifndef H5DATATYPE_HPP
#define H5DATATYPE_HPP

#include <boost/operators.hpp>

#include "H5Object.hpp"



namespace HighFive{

struct TypeMapper;

///
/// \brief HDF5 Data Type
///
class DataType : public Object, public boost::equality_comparable<DataType> {
public:
    DataType();

    bool operator==(const DataType & other) const;

protected:

    friend class File;
    friend class DataSet;
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


}

#include "bits/H5DataType_misc.hpp"

#endif // H5DATATYPE_HPP
