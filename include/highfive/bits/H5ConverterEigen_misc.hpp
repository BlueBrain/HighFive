/*
 *  Copyright (c), 2020, EPFL - Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <Eigen/Eigen>

namespace HighFive {

namespace details {

template <typename T, int M, int N>
struct manipulator<Eigen::Matrix<T, M, N>> {
    using type = Eigen::Matrix<T, M, N>;
    using value_type = typename type::value_type;
    using hdf5_type = typename manipulator<value_type>::hdf5_type;
    using h5_type = typename manipulator<value_type>::h5_type;

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{static_cast<size_t>(val.rows()), static_cast<size_t>(val.cols())};
    }

    static size_t flat_size(const type& val) {
        return static_cast<size_t>(val.size());
    }

    static hdf5_type* first(type& val) {
        return manipulator<value_type>::first(val.data()[0]);
    }

    static const hdf5_type* first(const type& val) {
        return manipulator<value_type>::first(val.data()[0]);
    }

    static value_type* data(type& val) {
        return val.data();
    }

    static const value_type* data(const type& val) {
        return val.data();
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() <= n_dims);

        val.resize(static_cast<typename type::Index>(count[0]),
                   static_cast<typename type::Index>(count.size() > 1 ? count[1] : 1));
    }

    static const size_t n_dims = 2 + manipulator<value_type>::n_dims;
};

template <typename T, int M, int N>
struct h5_continuous<Eigen::Matrix<T, M, N>> :
    std::true_type {};

}  // namespace details

}  // namespace HighFive
