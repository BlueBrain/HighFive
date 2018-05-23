/*
 *  Copyright (c), 2017-2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *                            Juan Hernando <juan.hernando@epfl.ch>
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5PROPERTY_LIST_HPP
#define H5PROPERTY_LIST_HPP

#include "H5Object.hpp"

#include <H5Ppublic.h>

namespace HighFive {

///
/// \brief Generic HDF5 property List
///
class Properties {
  public:
    enum Type
    {
       FILE_ACCESS,
       DATASET_CREATE
    };

    ~Properties();

    Type getType() const { return _type; }

    hid_t getId() const { return _hid; }

    /**
     * Add a property to this property list.
     * A property is an object which is expected to have a method with the
     * following signature void apply(hid_t hid) const
     */
    template <typename Property>
    void add(const Property& property);

  protected:

    // protected constructor
    Properties(Type type);

  private:
    Properties(const Properties&);
    Properties& operator=(const Properties&);

    Type _type;
    hid_t _hid;
};

} // HighFive

#include "bits/H5PropertyList_misc.hpp"

#endif // H5PROPERTY_LIST_HPP
