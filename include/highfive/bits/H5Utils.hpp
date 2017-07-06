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
#include <cstddef> // __GLIBCXX__
#include <exception>
#include <string>
#include <vector>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif

#ifndef H5_USE_CXX11
#if ___cplusplus >= 201103L
#define H5_USE_CXX11 1
#else
#define H5_USE_CXX11 0
#endif
#endif

// shared ptr portability
// if boost is used, simply use boost
#if (defined H5_USE_BOOST)
#include <boost/shared_ptr.hpp>
// if C++11 compliant compiler, use std::shared_ptr
#elif H5_USE_CXX11
#include <memory>
// GNU C++ or Intel C++ using libstd++.
// without C++11: use tr1
#elif defined(__GNUC__) && __GNUC__ >= 4 && (defined(__GLIBCXX__))
#include <tr1/memory>
// last hope to find it in standard <memory> ( VC++, libc++ )
#else
#include <memory>
#endif

namespace HighFive {

namespace details {

// determine at compile time number of dimensions of in memory datasets
template <typename T>
struct array_dims {
    static const size_t value = 0;
};

template <typename T>
struct array_dims<std::vector<T> > {
    static const size_t value = 1 + array_dims<T>::value;
};

template <typename T>
struct array_dims<T*> {
    static const size_t value = 1 + array_dims<T>::value;
};

template <typename T, std::size_t N>
struct array_dims<T[N]> {
    static const size_t value = 1 + array_dims<T>::value;
};

#ifdef H5_USE_BOOST
template <typename T, std::size_t Dims>
struct array_dims<boost::multi_array<T, Dims> > {
    static const size_t value = Dims;
};

template <typename T>
struct array_dims<boost::numeric::ublas::matrix<T> > {
    static const size_t value = 2;
};
#endif

// determine recursively the size of each dimension of a N dimension vector
template <typename T>
void get_dim_vector_rec(const T& vec, std::vector<size_t>& dims) {
    (void)dims;
    (void)vec;
}

template <typename T>
void get_dim_vector_rec(const std::vector<T>& vec, std::vector<size_t>& dims) {
    dims.push_back(vec.size());
    get_dim_vector_rec(vec[0], dims);
}

template <typename T>
std::vector<size_t> get_dim_vector(const std::vector<T>& vec) {
    std::vector<size_t> dims;
    get_dim_vector_rec(vec, dims);
    return dims;
}

// determine at compile time recursively the basic type of the data
template <typename T>
struct type_of_array {
    typedef T type;
};

template <typename T>
struct type_of_array<std::vector<T> > {
    typedef typename type_of_array<T>::type type;
};

#ifdef H5_USE_BOOST
template <typename T, std::size_t Dims>
struct type_of_array<boost::multi_array<T, Dims> > {
    typedef typename type_of_array<T>::type type;
};

template <typename T>
struct type_of_array<boost::numeric::ublas::matrix<T> > {
    typedef typename type_of_array<T>::type type;
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

// same type compile time check
template <typename T, typename U>
struct is_same {
    static const bool value = false;
};

template <typename T>
struct is_same<T, T> {
    static const bool value = true;
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
// check if the type is a container ( only vector supported for now )
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

// enable if implem for not c++11 compiler
template <bool Cond, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    typedef T type;
};

// remove const
template <typename Type>
struct remove_const {
    typedef Type type;
};

template <typename Type>
struct remove_const<Type const> {
    typedef Type type;
};

// shared ptr portability
namespace Mem {

#if (defined H5_USE_BOOST)
using namespace boost;
#elif ___cplusplus >= 201103L
using namespace std;
#elif defined(__GNUC__) && __GNUC__ >= 4 && (defined(__GLIBCXX__))
using namespace std::tr1;
#else
using namespace std;
#endif

} // end Mem

} // end details
}

#endif // H5UTILS_HPP
