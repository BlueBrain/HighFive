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

inline bool checkDimensions(const std::vector<size_t>& dims,
                            size_t min_dim_requested,
                            size_t max_dim_requested) {
    if (min_dim_requested <= dims.size() && dims.size() <= max_dim_requested) {
        return true;
    }


    // Scalar values still support broadcasting
    // into arrays with one element.
    size_t n_elements = compute_total_size(dims);
    return n_elements == 1 && min_dim_requested == 0;
}

}  // namespace details


/*****
inspector<T> {
    using type = T
    // base_type is the base type inside c++ (e.g. std::vector<int> => int)
    using base_type
    // hdf5_type is the base read by hdf5 (c-type) (e.g. std::vector<std::string> => const char*)
    using hdf5_type

    // Is the inner type trivially copyable for optimisation
    // If this value is true: data() is mandatory
    // If this value is false: serialize, unserialize are mandatory
    static constexpr bool is_trivially_copyable

    // Is this type trivially nestable, i.e. is type[n] a contiguous
    // array of `base_type[N]`?
    static constexpr bool is_trivially_nestable

    // Reading:
    // Allocate the value following dims (should be recursive)
    static void prepare(type& val, const std::vector<std::size_t> dims)
    // Return a pointer of the first value of val (for reading)
    static hdf5_type* data(type& val)
    // Take a serialized vector 'in', some dims and copy value to val (for reading)
    static void unserialize(const hdf5_type* in, const std::vector<size_t>&i, type& val)


    // Writing:
    // Return a point of the first value of val
    static const hdf5_type* data(const type& val)
    // Take a val and serialize it inside 'out'
    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* out)
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
    static constexpr size_t min_ndim = ndim;
    static constexpr size_t max_ndim = ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<type>::value;
    static constexpr bool is_trivially_nestable = is_trivially_copyable;

    static size_t getRank(const type& /* val */) {
        return ndim;
    }

    static std::vector<size_t> getDimensions(const type& /* val */) {
        return {};
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

    static void serialize(const type& val, const std::vector<size_t>& /* dims*/, hdf5_type* m) {
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
    static constexpr bool is_trivially_nestable = false;

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

    static void serialize(const type& val, const std::vector<size_t>& /* dims*/, hdf5_type* m) {
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
    static void serialize(const type& val, const std::vector<size_t>& /* dims*/, It m) {
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
    static constexpr bool is_trivially_nestable = false;

    static hdf5_type* data(type& /* val */) {
        throw DataSpaceException("A Reference cannot be read directly.");
    }

    static const hdf5_type* data(const type& /* val */) {
        throw DataSpaceException("A Reference cannot be written directly.");
    }

    static void serialize(const type& val, const std::vector<size_t>& /* dims*/, hdf5_type* m) {
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

template <typename T>
struct inspector<std::vector<T>> {
    using type = std::vector<T>;
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& val) {
        if (!val.empty()) {
            return ndim + inspector<value_type>::getRank(val[0]);
        } else {
            return min_ndim;
        }
    }

    static std::vector<size_t> getDimensions(const type& val) {
        auto rank = getRank(val);
        std::vector<size_t> sizes(rank, 1ul);
        sizes[0] = val.size();
        if (!val.empty()) {
            auto s = inspector<value_type>::getDimensions(val[0]);
            for (size_t i = 0; i < s.size(); ++i) {
                sizes[i + ndim] = s[i];
            }
        }
        return sizes;
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
    static void serialize(const type& val, const std::vector<size_t>& dims, It m) {
        if (!val.empty()) {
            auto subdims = std::vector<size_t>(dims.begin() + 1, dims.end());
            size_t subsize = compute_total_size(subdims);
            for (auto&& e: val) {
                inspector<value_type>::serialize(e, subdims, m);
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
    static constexpr size_t min_ndim = ndim;
    static constexpr size_t max_ndim = ndim;

    static constexpr bool is_trivially_copyable = false;
    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& /* val */) {
        return ndim;
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{val.size()};
        return sizes;
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

    static void serialize(const type& val, const std::vector<size_t>& /* dims*/, hdf5_type* m) {
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
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = (sizeof(type) == N * sizeof(T)) &&
                                                  is_trivially_copyable;

    static size_t getRank(const type& val) {
        return ndim + inspector<value_type>::getRank(val[0]);
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{N};
        auto s = inspector<value_type>::getDimensions(val[0]);
        sizes.insert(sizes.end(), s.begin(), s.end());
        return sizes;
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
    static void serialize(const type& val, const std::vector<size_t>& dims, It m) {
        auto subdims = std::vector<size_t>(dims.begin() + 1, dims.end());
        size_t subsize = compute_total_size(subdims);
        for (auto& e: val) {
            inspector<value_type>::serialize(e, subdims, m);
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
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& val) {
        if (val != nullptr) {
            return ndim + inspector<value_type>::getRank(val[0]);
        } else {
            return min_ndim;
        }
    }

    static std::vector<size_t> getDimensions(const type& /* val */) {
        throw DataSpaceException("Not possible to have size of a T*");
    }

    static const hdf5_type* data(const type& val) {
        return reinterpret_cast<const hdf5_type*>(val);
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static void serialize(const type& /* val */,
                          const std::vector<size_t>& /* dims*/,
                          hdf5_type* /* m */) {
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
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = is_trivially_copyable;

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

    static size_t getRank(const type& val) {
        return ndim + inspector<value_type>::getRank(val[0]);
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{N};
        auto s = inspector<value_type>::getDimensions(val[0]);
        sizes.insert(sizes.end(), s.begin(), s.end());
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
    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* m) {
        auto subdims = std::vector<size_t>(dims.begin() + 1, dims.end());
        size_t subsize = compute_total_size(subdims);
        for (size_t i = 0; i < N; ++i) {
            inspector<value_type>::serialize(val[i], subdims, m + i * subsize);
        }
    }
};


}  // namespace details
}  // namespace HighFive
