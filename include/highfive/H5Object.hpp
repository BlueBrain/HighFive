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
#ifndef H5OBJECT_HPP
#define H5OBJECT_HPP


#include <H5Ipublic.h>

namespace HighFive{

template <typename Derivate> class NodeTraits;

class Object{
public:

    ///
    /// \brief isValid
    /// \return true if current Object is a valid HDF5Object
    ///
    bool isValid() const;


    ///
    /// \brief getId
    /// \return iternal HDF5 id to the object
    ///  provided for C API compatibility
    ///
    hid_t getId()  const;


protected:
    // empty constructor
    Object();

    // copy constructor, increase reference counter
    Object(const Object & other);

    // decrease reference counter
    virtual ~Object();

    // override object destruction phase
    // needed for specific HDF5 type
    virtual void destroy();

    hid_t _hid;
private:
    template <typename Derivate>
    friend class NodeTraits;
};


}



#include "bits/H5Object_misc.hpp"

#endif // H5OBJECT_HPP
