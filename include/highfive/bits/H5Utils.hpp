/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5UTILS_HPP
#define H5UTILS_HPP

// internal utilities functions
#include <algorithm>
#include <array>
#include <cstddef> // __GLIBCXX__
#include <exception>
#include <string>
#include <type_traits>
#include <vector>

#ifdef H5_USE_BOOST
# include <boost/multi_array.hpp>
# include <boost/numeric/ublas/matrix.hpp>
#endif
#ifdef H5_USE_EIGEN
# include <Eigen/Eigen>
#endif

#include <H5public.h>

#include "../H5Exception.hpp"

namespace HighFive {

// If ever used, recognize dimensions of FixedLenStringArray
template <std::size_t N>
class FixedLenStringArray;


namespace details {
template <typename T>
struct manipulator {
    using type = T;

    static const size_t n_dims = 0;
    static const size_t r_n_dims = n_dims;
};

template <size_t N>
struct manipulator<FixedLenStringArray<N>> {
    using type = FixedLenStringArray<N>;

    static const size_t n_dims = 1;
    static const size_t r_n_dims = n_dims;
};

template <typename T>
struct manipulator<std::vector<T>> {
    using type = std::vector<T>;
    using value_type = T;

    static const size_t n_dims = 1;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};

template <typename T>
struct manipulator<T*> {
    using type = T*;
    using value_type = T;

    static const size_t n_dims = 1;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};

template <typename T, size_t N>
struct manipulator<T[N]> {
    using type = T[N];
    using value_type = T;

    static const size_t n_dims = 1;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};

template <typename T, size_t N>
struct manipulator<std::array<T, N>> {
    using type = std::array<T, N>;
    using value_type = T;

    static const size_t n_dims = 1;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};

#ifdef H5_USE_EIGEN
template <typename T, int M, int N>
struct manipulator<Eigen::Matrix<T, M, N>> {
    using type = Eigen::Matrix<T, M, N>;
    using value_type = T;

    static const size_t n_dims = 2;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};
#endif

#ifdef H5_USE_BOOST
template <typename T, size_t Dims>
struct manipulator<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = T;

    static const size_t n_dims = Dims;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};

template <typename T>
struct manipulator<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = T;

    static const size_t n_dims = 2;
    static const size_t r_n_dims = n_dims + manipulator<value_type>::r_n_dims;
};
#endif


// determine recursively the size of each dimension of a N dimension vector
template <typename T>
inline void get_dim_vector_rec(const T& /*vec*/, std::vector<size_t>& /*dims*/) {}

template <typename T>
inline void get_dim_vector_rec(const std::vector<T>& vec, std::vector<size_t>& dims) {
    dims.push_back(vec.size());
    get_dim_vector_rec(vec[0], dims);
}

template <typename T>
inline std::vector<size_t> get_dim_vector(const std::vector<T>& vec) {
    std::vector<size_t> dims;
    get_dim_vector_rec(vec, dims);
    return dims;
}

// determine recursively the size of each dimension of a N dimension vector
template <typename T, std::size_t N>
inline void get_dim_vector_rec(const T(&vec)[N], std::vector<size_t>& dims) {
    dims.push_back(N);
    get_dim_vector_rec(vec[0], dims);
}

template <typename T, std::size_t N>
inline std::vector<size_t> get_dim_vector(const T(&vec)[N]) {
    std::vector<size_t> dims;
    get_dim_vector_rec(vec, dims);
    return dims;
}


template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type
        >::type;

// determine at compile time recursively the basic type of the data
template <typename T>
struct type_of_array {
    typedef unqualified_t<T> type;
};

template <typename T>
struct type_of_array<std::vector<T>> {
    typedef typename type_of_array<T>::type type;
};

template <typename T, std::size_t N>
struct type_of_array<std::array<T, N>> {
    typedef typename type_of_array<T>::type type;
};

#ifdef H5_USE_BOOST
template <typename T, std::size_t Dims>
struct type_of_array<boost::multi_array<T, Dims>> {
    typedef typename type_of_array<T>::type type;
};

template <typename T>
struct type_of_array<boost::numeric::ublas::matrix<T>> {
    typedef typename type_of_array<T>::type type;
};
#endif

#ifdef H5_USE_EIGEN
template<typename T, int M, int N>
struct type_of_array<Eigen::Matrix<T, M, N>> {
    typedef T type;
};
#endif

template <typename T>
struct type_of_array<T*> {
    typedef typename type_of_array<T>::type type;
};

template <typename T, std::size_t N>
struct type_of_array<T[N]> {
    typedef typename type_of_array<T>::type type;
};


// Find the type of an eventual char array, otherwise void
template <typename>
struct type_char_array {
    typedef void type;
};

template <typename T>
struct type_char_array<T*> {
    typedef typename std::conditional<
        std::is_same<unqualified_t<T>, char>::value,
        char*,
        typename type_char_array<T>::type
    >::type type;
};

template <typename T, std::size_t N>
struct type_char_array<T[N]> {
    typedef typename std::conditional<
        std::is_same<unqualified_t<T>, char>::value,
        char[N],
        typename type_char_array<T>::type
    >::type type;
};


// check if the type is a container ( only vector supported for now )
template <typename>
struct is_container {
    static const bool value = false;
};

template <typename T>
struct is_container<std::vector<T> > {
    static const bool value = true;
};

// check if the type is a basic C-Array
template <typename>
struct is_c_array {
    static const bool value = false;
};

template <typename T>
struct is_c_array<T*> {
    static const bool value = true;
};

template <typename T, std::size_t N>
struct is_c_array<T[N]> {
    static const bool value = true;
};


// converter function for hsize_t -> size_t when hsize_t != size_t
template <typename Size>
inline std::vector<std::size_t> to_vector_size_t(const std::vector<Size>& vec) {
    static_assert(std::is_same<Size, std::size_t>::value == false,
                  " hsize_t != size_t mandatory here");
    std::vector<size_t> res(vec.size());
    std::transform(vec.cbegin(), vec.cend(), res.begin(), [](Size e) {
        return static_cast<size_t>(e);
    });
    return res;
}

// converter function for hsize_t -> size_t when size_t == hsize_t
inline std::vector<std::size_t> to_vector_size_t(const std::vector<std::size_t>& vec) {
    return vec;
}

// read name from a H5 object using the specified function
template<typename T>
inline std::string get_name(T fct) {
    const size_t maxLength = 255;
    char buffer[maxLength + 1];
    ssize_t retcode = fct(buffer, static_cast<hsize_t>(maxLength) + 1);
    if (retcode < 0) {
        HDF5ErrMapper::ToException<GroupException>("Error accessing object name");
    }
    const size_t length = static_cast<std::size_t>(retcode);
    if (length <= maxLength) {
        return std::string(buffer, length);
    }
    std::vector<char> bigBuffer(length + 1, 0);
    fct(bigBuffer.data(), static_cast<hsize_t>(length) + 1);
    return std::string(bigBuffer.data(), length);
}

}  // namespace details
}  // namespace HighFive

#endif // H5UTILS_HPP
