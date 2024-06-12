/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <complex>
#include <random>
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <functional>
#include <iomanip>

// We don't need windows specific functionality. However, to better detect defects caused by macros,
// we include this header.
// The list of identifiers is taken from `Boost::Predef`.
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__)
#define NOMINMAX
#include <Windows.h>
#endif

using ldcomplex = std::complex<long double>;
using dcomplex = std::complex<double>;
using fcomplex = std::complex<float>;

using base_test_types = std::tuple<int,
                                   unsigned int,
                                   long,
                                   unsigned long,
                                   unsigned char,
                                   char,
                                   float,
                                   double,
                                   long long,
                                   unsigned long long,
                                   ldcomplex,
                                   dcomplex,
                                   fcomplex>;

#ifdef HIGHFIVE_TEST_HALF_FLOAT
#include <highfive/half_float.hpp>

using float16_t = half_float::half;
using numerical_test_types =
    decltype(std::tuple_cat(std::declval<base_test_types>(), std::tuple<float16_t>()));
#else
using numerical_test_types = base_test_types;
#endif

using dataset_test_types =
    std::tuple<int, unsigned int, long, unsigned long, unsigned char, char, float, double>;


template <typename T, typename Func>
void fillVec(std::vector<std::vector<T>>& v, std::vector<size_t> dims, const Func& f) {
    v.resize(dims[0]);
    dims.erase(dims.begin());
    for (auto& subvec: v) {
        fillVec(subvec, dims, f);
    }
}

template <typename T, typename Func>
void fillVec(std::vector<T>& v, std::vector<size_t> dims, const Func& f) {
    v.resize(dims[0]);
    std::generate(v.begin(), v.end(), f);
}


template <typename T>
bool checkLength(const std::vector<T>& v, std::vector<size_t> dims) {
    return (dims.size() == 1 && v.size() == dims[0]);
}

template <typename T>
bool checkLength(const std::vector<std::vector<T>>& v, std::vector<size_t> dims) {
    size_t dim0 = dims[0];
    dims.erase(dims.begin());
    if (v.size() != dim0) {
        return false;
    }
    return checkLength(v[0], dims);
}


template <typename T, typename Func>
void generate2D(T* table, size_t x, size_t y, Func& func) {
    for (size_t i = 0; i < x; i++) {
        for (size_t j = 0; j < y; j++) {
            table[i][j] = func();
        }
    }
}


template <typename T>
struct ContentGenerate {
    ContentGenerate()
        : _init(0)
        , _inc(T(1) + T(1) / T(10)) {}

    T operator()() {
        T ret = _init;
        _init = static_cast<T>(_init + _inc);
        return ret;
    }

    T _init, _inc;
};

template <>
ContentGenerate<ldcomplex>::ContentGenerate()
    : _init(0, 0)
    , _inc(ldcomplex(1, 1) + ldcomplex(1, 1) / ldcomplex(10)) {}

template <>
ContentGenerate<dcomplex>::ContentGenerate()
    : _init(0, 0)
    , _inc(dcomplex(1, 1) + dcomplex(1, 1) / dcomplex(10)) {}

template <>
ContentGenerate<fcomplex>::ContentGenerate()
    : _init(0, 0)
    , _inc(fcomplex(1, 1) + fcomplex(1, 1) / fcomplex(10)) {}

template <>
struct ContentGenerate<char> {
    ContentGenerate()
        : _init('a') {}

    char operator()() {
        char ret = _init;
        if (++_init >= ('a' + 26))
            _init = 'a';
        return ret;
    }

    char _init;
};

template <>
struct ContentGenerate<std::string> {
    ContentGenerate() {}

    std::string operator()() {
        ContentGenerate<char> gen;
        std::string random_string;
        std::mt19937_64 rgen;
        rgen.seed(42);
        std::uniform_int_distribution<unsigned> int_dist(0, 1000);
        const size_t size_string = int_dist(rgen);

        random_string.resize(size_string);
        std::generate(random_string.begin(), random_string.end(), gen);
        return random_string;
    }
};


template <typename T>
inline std::string typeNameHelper() {
    std::string name = typeid(T).name();
    std::replace(std::begin(name), std::end(name), ' ', '_');
    std::replace(std::begin(name), std::end(name), '<', '_');
    std::replace(std::begin(name), std::end(name), '>', '_');
    std::replace(std::begin(name), std::end(name), ':', '_');

    if (name.size() > 64) {
        std::stringstream hash;
        hash << std::hex << std::hash<std::string>{}(name);

        return hash.str();
    } else {
        return name;
    }
}


template <typename ElemT, typename DataT>
inline HighFive::DataSet readWriteDataset(const DataT& ndvec,
                                          DataT& result,
                                          const size_t ndims,
                                          const std::string& struct_t) {
    using namespace HighFive;
    const std::string DATASET_NAME("dset");

    std::ostringstream filename;
    filename << "h5_rw_" << struct_t << "_" << ndims << "d_" << typeNameHelper<ElemT>()
             << "_test.h5";

    // Create a new file using the default property lists.
    File file(filename.str(), File::Truncate);

    // Create a dataset with type T points
    DataSet dataset = file.createDataSet<ElemT>(DATASET_NAME, DataSpace::From(ndvec));
    dataset.write(ndvec);

    dataset.read(result);
    return dataset;
}
