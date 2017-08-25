/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5PROPERTY_LIST_HPP
#define H5PROPERTY_LIST_HPP

#include "H5Object.hpp"

namespace HighFive {

///
/// \brief Generic HDF5 property List
///
class Properties : public Object {
  public:
  protected:
    // protected constructor
    inline Properties();

  private:
};

} // HighFive

#include "bits/H5PropertyList_misc.hpp"

#endif // H5PROPERTY_LIST_HPP
