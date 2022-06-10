/*
 *  Copyright (c) 2022 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <type_traits>
#include <cstring>

#include "../H5Reference.hpp"
#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
// starting Boost 1.64, serialization header must come before ublas
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif
#ifdef H5_USE_EIGEN
#include <Eigen/Eigen>
#endif

namespace HighFive {
inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}
template <size_t N>
inline size_t compute_total_size(const std::array<size_t, N>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;

template <typename T>
class Writer {
  public:
    const T* get_pointer() {
        if (vec.empty()) {
            return ptr;
        } else {
            return vec.data();
        }
    }
    std::vector<T> vec{};
    const T* ptr{nullptr};
};

template <typename T>
class Reader {
  public:
    T* get_pointer() {
        if (vec.empty()) {
            return ptr;
        } else {
            return vec.data();
        }
    }
    size_t get_size() {
        if (vec.empty()) {
            return size;
        } else {
            return vec.size();
        }
    }
    std::vector<T> vec{};
    size_t size{0};
    T* ptr{nullptr};
};

namespace details {
template <typename T>
struct type_helper {
    using type = T;
    using base_type = unqualified_t<T>;
    using hdf5_type = base_type;

    static constexpr size_t ndim = 0;
    static constexpr size_t recursive_ndim = ndim;
    static constexpr bool is_trivially_copyable = true;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        return {};
    }

    static size_t getSize(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& /* dims */) {
        Reader<hdf5_type> r;
        r.vec.resize(1);
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return &val;
    }

    static void serialize(const type& val, hdf5_type* m) {
        *m = val;
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& /* dims */) {
        return type{vec[0]};
    }
};

template <typename T>
struct inspector: type_helper<T> {};

template <>
struct inspector<std::string>: type_helper<std::string> {
    using hdf5_type = const char*;

    static constexpr bool is_trivially_copyable = false;

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& /* dims */) {
        Reader<hdf5_type> r;
        r.vec.resize(1);
        return r;
    }

    static void serialize(const type& val, hdf5_type* m) {
        *m = val.c_str();
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& /* dims */) {
        return type{vec[0]};
    }
};

template <>
struct inspector<Reference>: type_helper<Reference> {
    using hdf5_type = hobj_ref_t;

    static constexpr bool is_trivially_copyable = false;

    static void serialize(const type& val, hdf5_type* m) {
        hobj_ref_t ref;
        val.create_ref(&ref);
        *m = ref;
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& /* dims */) {
        return type{vec[0]};
    }
};

template <size_t N>
struct inspector<FixedLenStringArray<N>> {
    using type = FixedLenStringArray<N>;
    using base_type = FixedLenStringArray<N>;
    using hdf5_type = char;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim;
    static constexpr bool is_trivially_copyable = false;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        return std::array<size_t, recursive_ndim>{val.size()};
    }

    static size_t getSize(const type& val) {
        return N * compute_total_size(getDimensions(val));
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(N * compute_total_size(dims));
        return r;
    }

    static void serialize(const type& val, hdf5_type* m) {
        for (size_t i = 0; i < val.size(); ++i) {
            std::memcpy(m + i * N, val[i], N);
        }
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& dims) {
        type val = type{};
        for (size_t i = 0; i < dims[0]; ++i) {
            std::array<char, N> s;
            std::memcpy(s.data(), vec + (i * N), N);
            val.push_back(s);
        }
        return val;
    }
};

template <typename T>
struct inspector<std::vector<T>> {
    using type = std::vector<T>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{val.size()};
        size_t index = ndim;
        if (!val.empty()) {
            for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
                sizes[index++] = s;
            }
        }
        return sizes;
    }

    static size_t getSize(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(dims[0]);
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(val[0]);
    }

    static void serialize(const type& val, hdf5_type* m) {
        size_t subsize = compute_total_size(inspector<value_type>::getDimensions(val[0]));
        for (auto& e: val) {
            inspector<value_type>::serialize(e, m);
            m += subsize;
        }
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        type val{};
        prepare(val, dims);
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        size_t next_size = compute_total_size(next_dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            val[i] = inspector<value_type>::unserialize(vec_align + i * next_size, next_dims);
        }
        return val;
    }
};


template <typename T, size_t N>
struct inspector<std::array<T, N>> {
    using type = std::array<T, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{N};
        size_t index = ndim;
        if (!val.empty()) {
            for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
                sizes[index++] = s;
            }
        }
        return sizes;
    }

    static size_t getSize(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(val[0]);
    }

    static void serialize(const type& val, hdf5_type* m) {
        size_t subsize = compute_total_size(inspector<value_type>::getDimensions(val[0]));
        for (auto& e: val) {
            inspector<value_type>::serialize(e, m);
            m += subsize;
        }
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims[0] != N) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims[0] << " elements into an array with "
               << N << " elements.";
            throw DataSpaceException(os.str());
        }
        type val{};
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        size_t next_size = compute_total_size(next_dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            val[i] = inspector<value_type>::unserialize(vec_align + i * next_size, next_dims);
        }
        return val;
    }
};

template <typename T>
struct inspector<T*> {
    using type = T*;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = true;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        throw std::string("Not possible to have size of a T*");
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return reinterpret_cast<const hdf5_type*>(val);
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static void serialize(const type& val, hdf5_type* m) {
        m = reinterpret_cast<const hdf5_type*>(val);
    }
};

template <typename T>
struct inspector<const T*> {
    using type = const T*;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = true;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        throw std::string("Not possible to have size of a T*");
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return reinterpret_cast<const hdf5_type*>(val);
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static void serialize(const type& val, const hdf5_type* m) {
        m = reinterpret_cast<const hdf5_type*>(val);
    }
};


template <typename T, size_t N>
struct inspector<T[N]> {
    using type = T[N];
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = true;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{N};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return reinterpret_cast<const hdf5_type*>(val);
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static void serialize(const type& val, const hdf5_type* m) {
        m = reinterpret_cast<const hdf5_type*>(&val[0]);
    }
};

#ifdef H5_USE_EIGEN
template <typename T, int M, int N>
struct inspector<Eigen::Matrix<T, M, N>> {
    using type = Eigen::Matrix<T, M, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{static_cast<size_t>(val.rows()),
                                                 static_cast<size_t>(val.cols())};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val.data()[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }

    static size_t getSize(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(static_cast<typename type::Index>(dims[0]),
                   static_cast<typename type::Index>(dims[1]));
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(*val.data());
    }

    static void serialize(const type& val, hdf5_type* m) {
        std::memcpy(m, val.data(), static_cast<size_t>(val.size()) * sizeof(hdf5_type));
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims.size() < 2) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size()
               << " dimensions into an eigen-matrix.";
            throw DataSpaceException(os.str());
        }
        type array{};
        prepare(array, dims);
        std::memcpy(array.data(), vec_align, compute_total_size(dims) * sizeof(hdf5_type));
        return array;
    }
};
#endif

#ifdef H5_USE_BOOST
template <typename T, size_t Dims>
struct inspector<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = Dims;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes;
        for (size_t i = 0; i < ndim; ++i) {
            sizes[i] = val.shape()[i];
        }

        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val.data()[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }

    static size_t getSize(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        boost::array<typename type::index, Dims> ext;
        std::copy(dims.begin(), dims.begin() + ndim, ext.begin());
        val.resize(ext);
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(*val.data());
    }

    static void serialize(const type& val, hdf5_type* m) {
        size_t size = val.num_elements();
        size_t subsize = compute_total_size(inspector<value_type>::getDimensions(*val.origin()));
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::serialize(*(val.origin() + i), m + i * subsize);
        }
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims.size() < ndim) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size() << " dimensions into a " << ndim
               << " boost::multi-array.";
            throw DataSpaceException(os.str());
        }
        type array{};
        prepare(array, dims);
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        for (size_t i = 0; i < array.num_elements(); ++i) {
            *(array.origin() + i) = inspector<value_type>::unserialize(vec_align + i * subsize,
                                                                       next_dims);
        }
        return array;
    }
};

template <typename T>
struct inspector<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{val.size1(), val.size2()};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val(0, 0))) {
            sizes[index++] = s;
        }
        return sizes;
    }

    static size_t getSize(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(dims[0], dims[1], false);
    }

    static Reader<hdf5_type> get_reader(const std::vector<size_t>& dims) {
        Reader<hdf5_type> r;
        r.vec.resize(compute_total_size(dims));
        return r;
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(val(0, 0));
    }

    static void serialize(const type& val, hdf5_type* m) {
        size_t size = val.size1() * val.size2();
        size_t subsize = compute_total_size(inspector<value_type>::getDimensions(val(0, 0)));
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::serialize(*(&val(0, 0) + i), m + i * subsize);
        }
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims.size() < 2) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size() << " dimensions into a " << ndim
               << " boost::numeric::ublas::matrix";
            throw DataSpaceException(os.str());
        }
        type array{};
        prepare(array, dims);
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        size_t size = array.size1() * array.size2();
        for (size_t i = 0; i < size; ++i) {
            *(&array(0, 0) + i) = inspector<value_type>::unserialize(vec_align + i * subsize,
                                                                     next_dims);
        }
        return array;
    }
};
#endif

struct data_converter {
    template <typename T>
    static typename std::enable_if<inspector<T>::is_trivially_copyable,
                                   Writer<typename inspector<T>::hdf5_type>>::type
    serialize(const typename inspector<T>::type& val) {
        using hdf5_type = typename inspector<T>::hdf5_type;
        Writer<hdf5_type> w;
        w.ptr = inspector<T>::data(val);
        return w;
    }

    template <typename T>
    static typename std::enable_if<!inspector<T>::is_trivially_copyable,
                                   Writer<typename inspector<T>::hdf5_type>>::type
    serialize(const typename inspector<T>::type& val) {
        using hdf5_type = typename inspector<T>::hdf5_type;
        Writer<hdf5_type> w;
        w.vec.resize(inspector<T>::getSize(val));
        inspector<T>::serialize(val, w.vec.data());
        return w;
    }
};

}  // namespace details
}  // namespace HighFive
