/*
 *  Copyright (c), 2020, EPFL - Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef H5REFERENCE_HPP
#define H5REFERENCE_HPP

#include <H5Rpublic.h>


namespace HighFive {

///
/// \brief Create an HDF5 (object) reference type
///

class Reference {
  public:
    Reference() = default;

    Reference(const Object& parent, const Object& o);

    explicit Reference(const hobj_ref_t h5_ref)
        : href(h5_ref){};

    void create_ref(hobj_ref_t* refptr) const;

    template <typename T>
    T dereference(const Object& loc);
    // Group dereference(const Object& loc);
  private:
    hobj_ref_t href{};
    std::string obj_name;
    hid_t parent_id{};
};

}

#include "H5DataSet.hpp"
#include "bits/H5Reference_misc.hpp"

#endif // H5REFERENCE_HPP