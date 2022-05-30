/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5CONVERTER_MISC_HPP
#define H5CONVERTER_MISC_HPP

#include <H5Dpublic.h>
#include <H5Ppublic.h>

namespace HighFive {

namespace details {

inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

// DATA CONVERTERS
// ===============

// apply conversion operations to basic scalar type
template <typename T, class Enable>
struct data_converter {
    using hdf5_type = typename inspector<T>::hdf5_type;
    inline data_converter(const DataSpace& space)
        : _dims(space.getDimensions())
        , _space(space) {}

    inline hdf5_type* transform_read(T&) {
        _vec_align.resize(compute_total_size(_dims));
        return _vec_align.data();
    }

    inline void process_result(T& val) {
        inspector<T>::prepare(val, _dims);
        val = inspector<T>::unserialize(_vec_align.data(), _dims);
        auto t = create_datatype<typename inspector<T>::base_type>();
        auto c = t.getClass();
        if (c == DataTypeClass::VarLen) {
            (void) H5Dvlen_reclaim(t.getId(), _space.getId(), H5P_DEFAULT, &val);
        }
    }

    std::vector<size_t> _dims;
    DataSpace _space;
    std::vector<hdf5_type> _vec_align;
};


// apply conversion operations to the incoming data
// if they are a cstyle array
template <typename CArray>
struct data_converter<CArray, typename std::enable_if<(is_c_array<CArray>::value)>::type> {
    using hdf5_type = typename inspector<CArray>::hdf5_type;
    inline data_converter(const DataSpace&) noexcept {}

    inline CArray& transform_read(CArray& datamem) const noexcept {
        return datamem;
    }

    inline void process_result(CArray&) const noexcept {}
};

}  // namespace details

}  // namespace HighFive

#endif  // H5CONVERTER_MISC_HPP
