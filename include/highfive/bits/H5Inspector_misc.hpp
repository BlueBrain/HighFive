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
#include <cassert>
#include <vector>
#include <array>
#include <string>
#include <numeric>

#include "../H5Reference.hpp"

#include "string_padding.hpp"

#include "H5Inspector_decl.hpp"


namespace HighFive {
namespace details {

inline bool checkDimensions(const std::vector<size_t>& dims, size_t n_dim_requested) {
    size_t n_dim_actual = dims.size();

    // We should allow reading scalar from shapes like `(1, 1, 1)`.
    if (n_dim_requested == 0) {
        if (n_dim_actual == 0ul) {
            return true;
        }

        return size_t(std::count(dims.begin(), dims.end(), 1ul)) == n_dim_actual;
    }

    // For non-scalar datasets, we can squeeze away singleton dimension, but
    // we never add any.
    if (n_dim_actual < n_dim_requested) {
        return false;
    }

    // Special case for 1-dimensional arrays, which can squeeze `1`s from either
    // side simultaneously if needed.
    if (n_dim_requested == 1ul) {
        return n_dim_actual >= 1ul &&
               size_t(std::count(dims.begin(), dims.end(), 1ul)) >= n_dim_actual - 1ul;
    }

    // All other cases strip front only. This avoid unstable behaviour when
    // squeezing singleton dimensions.
    size_t n_dim_excess = n_dim_actual - n_dim_requested;

    bool squeeze_back = true;
    for (size_t i = 1; i <= n_dim_excess; ++i) {
        if (dims[n_dim_actual - i] != 1) {
            squeeze_back = false;
            break;
        }
    }

    return squeeze_back;
}


inline std::vector<size_t> squeezeDimensions(const std::vector<size_t>& dims,
                                             size_t n_dim_requested) {
    auto format_error_message = [&]() -> std::string {
        return "Can't interpret dims = " + format_vector(dims) + " as " +
               std::to_string(n_dim_requested) + "-dimensional.";
    };

    if (n_dim_requested == 0) {
        if (!checkDimensions(dims, n_dim_requested)) {
            throw std::invalid_argument("Failed dimensions check: " + format_error_message());
        }

        return {1ul};
    }

    auto n_dim = dims.size();
    if (n_dim < n_dim_requested) {
        throw std::invalid_argument("Failed 'n_dim < n_dim_requested: " + format_error_message());
    }

    if (n_dim_requested == 1ul) {
        size_t non_singleton_dim = size_t(-1);
        for (size_t i = 0; i < n_dim; ++i) {
            if (dims[i] != 1ul) {
                if (non_singleton_dim == size_t(-1)) {
                    non_singleton_dim = i;
                } else {
                    throw std::invalid_argument("Failed one-dimensional: " +
                                                format_error_message());
                }
            }
        }

        return {dims[std::min(non_singleton_dim, n_dim - 1)]};
    }

    size_t n_dim_excess = dims.size() - n_dim_requested;
    for (size_t i = 1; i <= n_dim_excess; ++i) {
        if (dims[n_dim - i] != 1) {
            throw std::invalid_argument("Failed stripping from back:" + format_error_message());
        }
    }

    return std::vector<size_t>(dims.begin(),
                               dims.end() - static_cast<std::ptrdiff_t>(n_dim_excess));
}
}  // namespace details


/*****
inspector<T> {
    using type = T
    // base_type is the base type inside c++ (e.g. std::vector<int> => int)
    using base_type
    // hdf5_type is the base read by hdf5 (c-type) (e.g. std::vector<std::string> => const char*)
    using hdf5_type

    // Number of dimensions starting from here
    static constexpr size_t recursive_ndim
    // Is the inner type trivially copyable for optimisation
    // If this value is true: data() is mandatory
    // If this value is false: getSizeVal, getSize, serialize, unserialize are mandatory
    static constexpr bool is_trivially_copyable

    // Reading:
    // Allocate the value following dims (should be recursive)
    static void prepare(type& val, const std::vector<std::size_t> dims)
    // Return the size of the vector pass to/from hdf5 from a vector of dims
    static size_t getSize(const std::vector<size_t>& dims)
    // Return a pointer of the first value of val (for reading)
    static hdf5_type* data(type& val)
    // Take a serialized vector 'in', some dims and copy value to val (for reading)
    static void unserialize(const hdf5_type* in, const std::vector<size_t>&i, type& val)


    // Writing:
    // Return the size of the vector pass to/from hdf5 from a value
    static size_t getSizeVal(const type& val)
    // Return a point of the first value of val
    static const hdf5_type* data(const type& val)
    // Take a val and serialize it inside 'out'
    static void serialize(const type& val, hdf5_type* out)
    // Return an array of dimensions of the space needed for writing val
    static std::vector<size_t> getDimensions(const type& val)
}
*****/


namespace details {
template <typename T>
struct type_helper {
    using type = unqualified_t<T>;
    using base_type = unqualified_t<T>;
    using hdf5_type = base_type;

    static constexpr size_t ndim = 0;
    static constexpr size_t recursive_ndim = ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<type>::value;

    static std::vector<size_t> getDimensions(const type& /* val */) {
        return {};
    }

    static size_t getSizeVal(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        return compute_total_size(dims);
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static hdf5_type* data(type& val) {
        static_assert(is_trivially_copyable, "The type is not trivially copyable");
        return &val;
    }

    static const hdf5_type* data(const type& val) {
        static_assert(is_trivially_copyable, "The type is not trivially copyable");
        return &val;
    }

    static void serialize(const type& val, hdf5_type* m) {
        static_assert(is_trivially_copyable, "The type is not trivially copyable");
        *m = val;
    }

    static void unserialize(const hdf5_type* vec,
                            const std::vector<size_t>& /* dims */,
                            type& val) {
        static_assert(is_trivially_copyable, "The type is not trivially copyable");
        val = vec[0];
    }
};

template <typename T>
struct inspector: type_helper<T> {};

enum class Boolean : int8_t {
    HighFiveFalse = 0,
    HighFiveTrue = 1,
};

template <>
struct inspector<bool>: type_helper<bool> {
    using base_type = Boolean;
    using hdf5_type = int8_t;

    static constexpr bool is_trivially_copyable = false;

    static hdf5_type* data(type& /* val */) {
        throw DataSpaceException("A boolean cannot be read directly.");
    }

    static const hdf5_type* data(const type& /* val */) {
        throw DataSpaceException("A boolean cannot be written directly.");
    }

    static void unserialize(const hdf5_type* vec,
                            const std::vector<size_t>& /* dims */,
                            type& val) {
        val = vec[0] != 0 ? true : false;
    }

    static void serialize(const type& val, hdf5_type* m) {
        *m = val ? 1 : 0;
    }
};

template <>
struct inspector<std::string>: type_helper<std::string> {
    using hdf5_type = const char*;

    static hdf5_type* data(type& /* val */) {
        throw DataSpaceException("A std::string cannot be read directly.");
    }

    static const hdf5_type* data(const type& /* val */) {
        throw DataSpaceException("A std::string cannot be written directly.");
    }

    template <class It>
    static void serialize(const type& val, It m) {
        (*m).assign(val.data(), val.size(), StringPadding::NullTerminated);
    }

    template <class It>
    static void unserialize(const It& vec, const std::vector<size_t>& /* dims */, type& val) {
        const auto& view = *vec;
        val.assign(view.data(), view.length());
    }
};

template <>
struct inspector<Reference>: type_helper<Reference> {
    using hdf5_type = hobj_ref_t;

    static constexpr bool is_trivially_copyable = false;

    static hdf5_type* data(type& /* val */) {
        throw DataSpaceException("A Reference cannot be read directly.");
    }

    static const hdf5_type* data(const type& /* val */) {
        throw DataSpaceException("A Reference cannot be written directly.");
    }

    static void serialize(const type& val, hdf5_type* m) {
        hobj_ref_t ref;
        val.create_ref(&ref);
        *m = ref;
    }

    static void unserialize(const hdf5_type* vec,
                            const std::vector<size_t>& /* dims */,
                            type& val) {
        val = type{vec[0]};
    }
};

template <size_t N>
struct inspector<deprecated::FixedLenStringArray<N>> {
    using type = deprecated::FixedLenStringArray<N>;
    using value_type = char*;
    using base_type = deprecated::FixedLenStringArray<N>;
    using hdf5_type = char;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim;
    static constexpr bool is_trivially_copyable = false;

    static std::vector<size_t> getDimensions(const type& val) {
        return std::vector<size_t>{val.size()};
    }

    static size_t getSizeVal(const type& val) {
        return N * compute_total_size(getDimensions(val));
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        return N * compute_total_size(dims);
    }

    static void prepare(type& /* val */, const std::vector<size_t>& dims) {
        if (dims[0] > N) {
            std::ostringstream os;
            os << "Size of FixedlenStringArray (" << N << ") is too small for dims (" << dims[0]
               << ").";
            throw DataSpaceException(os.str());
        }
    }

    static hdf5_type* data(type& val) {
        return val.data();
    }

    static const hdf5_type* data(const type& val) {
        return val.data();
    }

    static void serialize(const type& val, hdf5_type* m) {
        for (size_t i = 0; i < val.size(); ++i) {
            std::memcpy(m + i * N, val[i], N);
        }
    }

    static void unserialize(const hdf5_type* vec, const std::vector<size_t>& dims, type& val) {
        for (size_t i = 0; i < dims[0]; ++i) {
            std::array<char, N> s;
            std::memcpy(s.data(), vec + (i * N), N);
            val.push_back(s);
        }
    }
};

template <typename T>
struct inspector<std::vector<T>> {
    using type = std::vector<T>;
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes(recursive_ndim, 1ul);
        sizes[0] = val.size();
        if (!val.empty()) {
            auto s = inspector<value_type>::getDimensions(val[0]);
            assert(s.size() + ndim == sizes.size());
            for (size_t i = 0; i < s.size(); ++i) {
                sizes[i + ndim] = s[i];
            }
        }
        return sizes;
    }

    static size_t getSizeVal(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        return compute_total_size(dims);
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(dims[0]);
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        for (auto&& e: val) {
            inspector<value_type>::prepare(e, next_dims);
        }
    }

    static hdf5_type* data(type& val) {
        return val.empty() ? nullptr : inspector<value_type>::data(val[0]);
    }

    static const hdf5_type* data(const type& val) {
        return val.empty() ? nullptr : inspector<value_type>::data(val[0]);
    }

    template <class It>
    static void serialize(const type& val, It m) {
        if (!val.empty()) {
            size_t subsize = inspector<value_type>::getSizeVal(val[0]);
            for (auto&& e: val) {
                inspector<value_type>::serialize(e, m);
                m += subsize;
            }
        }
    }

    template <class It>
    static void unserialize(const It& vec_align, const std::vector<size_t>& dims, type& val) {
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        size_t next_size = compute_total_size(next_dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            inspector<value_type>::unserialize(vec_align + i * next_size, next_dims, val[i]);
        }
    }
};

template <>
struct inspector<std::vector<bool>> {
    using type = std::vector<bool>;
    using value_type = bool;
    using base_type = Boolean;
    using hdf5_type = uint8_t;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim;
    static constexpr bool is_trivially_copyable = false;

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{val.size()};
        return sizes;
    }

    static size_t getSizeVal(const type& val) {
        return val.size();
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        if (dims.size() > 1) {
            throw DataSpaceException("std::vector<bool> is only 1 dimension.");
        }
        return dims[0];
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims.size() > 1) {
            throw DataSpaceException("std::vector<bool> is only 1 dimension.");
        }
        val.resize(dims[0]);
    }

    static hdf5_type* data(type& /* val */) {
        throw DataSpaceException("A std::vector<bool> cannot be read directly.");
    }

    static const hdf5_type* data(const type& /* val */) {
        throw DataSpaceException("A std::vector<bool> cannot be written directly.");
    }

    static void serialize(const type& val, hdf5_type* m) {
        for (size_t i = 0; i < val.size(); ++i) {
            m[i] = val[i] ? 1 : 0;
        }
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        for (size_t i = 0; i < dims[0]; ++i) {
            val[i] = vec_align[i] != 0 ? true : false;
        }
    }
};

template <typename T, size_t N>
struct inspector<std::array<T, N>> {
    using type = std::array<T, N>;
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  sizeof(type) == N * sizeof(T) &&
                                                  inspector<value_type>::is_trivially_copyable;

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{N};
        if (!val.empty()) {
            auto s = inspector<value_type>::getDimensions(val[0]);
            sizes.insert(sizes.end(), s.begin(), s.end());
        }
        return sizes;
    }

    static size_t getSizeVal(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        return compute_total_size(dims);
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims[0] > N) {
            std::ostringstream os;
            os << "Size of std::array (" << N << ") is too small for dims (" << dims[0] << ").";
            throw DataSpaceException(os.str());
        }

        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        for (auto&& e: val) {
            inspector<value_type>::prepare(e, next_dims);
        }
    }

    static hdf5_type* data(type& val) {
        return inspector<value_type>::data(val[0]);
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(val[0]);
    }

    template <class It>
    static void serialize(const type& val, It m) {
        size_t subsize = inspector<value_type>::getSizeVal(val[0]);
        for (auto& e: val) {
            inspector<value_type>::serialize(e, m);
            m += subsize;
        }
    }

    template <class It>
    static void unserialize(const It& vec_align, const std::vector<size_t>& dims, type& val) {
        if (dims[0] != N) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims[0] << " elements into an array with "
               << N << " elements.";
            throw DataSpaceException(os.str());
        }
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        size_t next_size = compute_total_size(next_dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            inspector<value_type>::unserialize(vec_align + i * next_size, next_dims, val[i]);
        }
    }
};

// Cannot be use for reading
template <typename T>
struct inspector<T*> {
    using type = T*;
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;

    static size_t getSizeVal(const type& /* val */) {
        throw DataSpaceException("Not possible to have size of a T*");
    }

    static std::vector<size_t> getDimensions(const type& /* val */) {
        throw DataSpaceException("Not possible to have size of a T*");
    }

    static const hdf5_type* data(const type& val) {
        return reinterpret_cast<const hdf5_type*>(val);
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static void serialize(const type& /* val */, hdf5_type* /* m */) {
        throw DataSpaceException("Not possible to serialize a T*");
    }
};

// Cannot be use for reading
template <typename T, size_t N>
struct inspector<T[N]> {
    using type = T[N];
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims.size() < 1) {
            throw DataSpaceException("Invalid 'dims', must be at least 1 dimensional.");
        }

        if (dims[0] != N) {
            throw DataSpaceException("Dimensions mismatch.");
        }

        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        for (size_t i = 0; i < dims[0]; ++i) {
            inspector<value_type>::prepare(val[i], next_dims);
        }
    }

    static size_t getSizeVal(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{N};
        if (N > 0) {
            auto s = inspector<value_type>::getDimensions(val[0]);
            sizes.insert(sizes.end(), s.begin(), s.end());
        }
        return sizes;
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(val[0]);
    }

    static hdf5_type* data(type& val) {
        return inspector<value_type>::data(val[0]);
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static void serialize(const type& val, hdf5_type* m) {
        size_t subsize = inspector<value_type>::getSizeVal(val[0]);
        for (size_t i = 0; i < N; ++i) {
            inspector<value_type>::serialize(val[i], m + i * subsize);
        }
    }
};

}  // namespace details
}  // namespace HighFive

#ifdef H5_USE_BOOST
#include <highfive/boost.hpp>
#endif

#ifdef H5_USE_EIGEN
#include <highfive/eigen.hpp>
#endif
