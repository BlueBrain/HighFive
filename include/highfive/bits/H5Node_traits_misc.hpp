/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
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
inline size_t NodeTraits<Derivate>::getNumberObjects() const{
    hsize_t res;
    if ( H5Gget_num_objs(static_cast<const Derivate*>(this)->getId(), &res) < 0){
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to count objects in existing group or file"));
    }
    return res;
}


template <typename Derivate>
inline std::vector<std::string> NodeTraits<Derivate>::listObjectNames() const{
    size_t max_read_size = 4096;
    size_t i=0, num_objs = getNumberObjects();
    std::vector<std::string> names;
    names.reserve(num_objs);


    do{
        std::vector<char> buffer(max_read_size, 0);
        hid_t gid = static_cast<const Derivate*>(this)->getId();
        size_t sread;

        if((sread = H5Gget_objname_by_idx(gid, static_cast<hsize_t>(i),
            buffer.data(), max_read_size )) < 0){
            HDF5ErrMapper::ToException<GroupException>(std::string("Unable to list objects in existing group or file"));
        }

        if(sread >= max_read_size - 2){
            // buffer too short, truncated result
            // we enlarge the buffer
            max_read_size = std::max<ssize_t>(max_read_size*2,
                                     H5Gget_objname_by_idx(gid,
                                                           static_cast<hsize_t>(i),
                                                            NULL, 0 ));
            continue;
        }
        names.push_back(buffer.data());
        ++i;
    }while(i < num_objs);
    return names;
}

}

#endif // H5NODE_TRAITS_MISC_HPP
