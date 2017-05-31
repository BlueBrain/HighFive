/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5NODE_TRAITS_MISC_HPP
#define H5NODE_TRAITS_MISC_HPP

#include "H5Node_traits.hpp"
#include "H5Iterables_misc.hpp"

#include <string>
#include <vector>

#include "../H5Attribute.hpp"
#include "../H5Exception.hpp"
#include "../H5DataSet.hpp"
#include "../H5DataSpace.hpp"
#include "../H5DataType.hpp"
#include "../H5Group.hpp"



#include <H5Apublic.h>
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
inline DataSet NodeTraits<Derivate>::getDataSet(const std::string & dataset_name) const{
    DataSet set;
    if( (set._hid = H5Dopen2(static_cast<const Derivate*>(this)->getId(), dataset_name.c_str(), H5P_DEFAULT)) < 0){
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
inline Group NodeTraits<Derivate>::getGroup(const std::string & group_name) const{
    Group group;
    if( (group._hid = H5Gopen2(static_cast<const Derivate*>(this)->getId(), group_name.c_str(), H5P_DEFAULT)) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open the group \"")+ group_name + "\":");
    }
    return group;
}


template <typename Derivate>
inline bool NodeTraits<Derivate>::hasItem(const std::string & node_name) const{
    return H5Lexists(static_cast<const Derivate*>(this)->getId(), node_name.c_str(), H5P_DEFAULT);
}


template <typename Derivate>
inline size_t NodeTraits<Derivate>::getNumberObjects() const{
    hsize_t res;
    if ( H5Gget_num_objs(static_cast<const Derivate*>(this)->getId(), &res) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to count objects in existing group or file"));
    }
    return res;
}



template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listObjectNames() const{

    std::vector<std::string> names;
    details::HighFiveIterateData iterateData(names);


    size_t num_objs = getNumberObjects();
    names.reserve(num_objs);

    if( H5Literate(static_cast<const Derivate*>(this)->getId(), H5_INDEX_NAME,
                   H5_ITER_INC, NULL, &details::internal_high_five_iterate<H5L_info_t>, static_cast<void*>(&iterateData)) < 0){
         HDF5ErrMapper::ToException<GroupException>(std::string("Unable to list objects in group"));
    }

    return names;
}

}

#endif // H5NODE_TRAITS_MISC_HPP
