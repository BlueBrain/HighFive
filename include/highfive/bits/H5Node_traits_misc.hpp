#ifndef H5NODE_TRAITS_MISC_HPP
#define H5NODE_TRAITS_MISC_HPP

#include "H5Node_traits.hpp"

#include <string>

#include "../H5Exception.hpp"
#include "../H5DataSet.hpp"
#include "../H5DataSpace.hpp"
#include "../H5DataType.hpp"
#include "../H5Group.hpp"



#include <H5Dpublic.h>
#include <H5Tpublic.h>
#include <H5Fpublic.h>
#include <H5Ppublic.h>
#include <H5Gpublic.h>

namespace HighFive{

template <typename Derivate>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string & dataset_name, const DataSpace & space, const DataType & dtype){
    DataSet set;
    if( (set._hid = H5Dcreate2(static_cast<Derivate*>(this)->getId(), dataset_name.c_str(), dtype._hid, space._hid,
                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to create the dataset \"")+ dataset_name+ "\":");
    }
    return set;
}

template <typename Derivate>
template <typename Type>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string & dataset_name, const DataSpace & space){
    return createDataSet(dataset_name, space, AtomicType<Type>());
}

template <typename Derivate>
template <typename Vector>
inline DataSet NodeTraits<Derivate>::createDataSet(const std::string & dataset_name, const Vector & vector){
    return createDataSet(dataset_name, DataSpace(details::get_dim_vector<Vector>(vector)), AtomicType<typename details::type_of_array<Vector>::type>());
}

template <typename Derivate>
inline DataSet NodeTraits<Derivate>::getDataSet(const std::string & dataset_name){
    DataSet set;
    if( (set._hid = H5Dopen2(static_cast<Derivate*>(this)->getId(), dataset_name.c_str(), H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<DataSetException>(std::string("Unable to open the dataset \"")+ dataset_name + "\":");
    }
    return set;
}

template <typename Derivate>
inline Group NodeTraits<Derivate>::createGroup(const std::string & group_name){
    Group group;
    if( (group._hid = H5Gcreate2(static_cast<Derivate*>(this)->getId(), group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to create the group \"")+ group_name + "\":");
    }
    return group;
}



template <typename Derivate>
inline Group NodeTraits<Derivate>::getGroup(const std::string & group_name){
    Group group;
    if( (group._hid = H5Gopen2(static_cast<Derivate*>(this)->getId(), group_name.c_str(), H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open the group \"")+ group_name + "\":");
    }
    return group;
}

}

#endif // H5NODE_TRAITS_MISC_HPP
