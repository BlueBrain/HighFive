/*
 *  Copyright (c), 2020, EPFL - Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef H5REFERENCE_MISC_HPP
#define H5REFERENCE_MISC_HPP

#include <string>

namespace HighFive {

inline Reference::Reference(const Object& parent, const Object& o)
    : parent_id(parent.getId()) {

    const size_t maxLength = 255;
    char buffer[maxLength + 1];
    if (H5Iget_name(o.getId(), buffer, maxLength) <= 0) {
        HDF5ErrMapper::ToException<DataTypeException>("Invalid object or location");
    }
    obj_name = std::string(buffer);
}


void Reference::create_ref(hobj_ref_t* refptr) const {

    H5Rcreate(refptr, parent_id, obj_name.c_str(), H5R_OBJECT, -1);
}

template <typename T>
T Reference::dereference(const Object& loc) {
    static_assert(std::is_base_of<Object, T>::value != 0,
        "Can only return a subtype of HighFive::Object");
    hid_t res = H5Rdereference(loc.getId(), H5R_OBJECT, &href);
    auto obj = Object(res);
    T h5_obj(obj);
    return h5_obj;
}

namespace details {

// apply conversion for vectors nested vectors
template <>
struct data_converter<std::vector<Reference>> {
    inline data_converter(const std::vector<Reference>&, const DataSpace& space)
        : _dims(space.getDimensions()) {
        if (!is_1D(_dims)) {
            throw DataSpaceException("Only 1D std::array supported currently.");
        }
    }

    inline typename type_of_array<hobj_ref_t>::type*
    transform_read(std::vector<Reference>&) {
        _vec_align.resize(compute_total_size(_dims));
        return _vec_align.data();
    }

    inline typename type_of_array<hobj_ref_t>::type* transform_write(const std::vector<Reference>& vec) {
        _vec_align.reserve(compute_total_size(_dims));
        for (size_t i = 0; i < vec.size(); ++i) {
            vec[i].create_ref(&_vec_align[i]);
        }
        return _vec_align.data();
    }

    inline void process_result(std::vector<Reference>& vec) const {
        hobj_ref_t* href = const_cast<hobj_ref_t*>(_vec_align.data());
        for (auto& ref : vec) {
            ref = Reference(*(href++));
        }
    }

    std::vector<size_t> _dims;
    std::vector<typename type_of_array<hobj_ref_t>::type> _vec_align;
};


}

}


#endif  // H5REFERENCE_MISC_HPP
