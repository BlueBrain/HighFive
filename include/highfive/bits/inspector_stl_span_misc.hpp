#pragma once

#include "H5Inspector_decl.hpp"
#include "../H5Exception.hpp"

#include <cstdlib>
#include <vector>
#include <type_traits>

namespace HighFive {
namespace details {


// Anything with the same API as `std::span` can implemented by inheriting from
// this class.
template <class Span>
struct inspector_stl_span {
    using type = Span;
    using value_type = unqualified_t<typename Span::value_type>;
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
            assert(s.size() + ndim == sizes.size());
            for (size_t i = 0; i < s.size(); ++i) {
                sizes[i + ndim] = s[i];
            }
        }
        return sizes;
    }

    static void prepare(type& val, const std::vector<size_t>& expected_dims) {
        auto actual_dims = getDimensions(val);
        if (actual_dims.size() != expected_dims.size()) {
            throw DataSpaceException("Mismatching rank.");
        }

        for (size_t i = 0; i < actual_dims.size(); ++i) {
            if (actual_dims[i] != expected_dims[i]) {
                throw DataSpaceException("Mismatching dimensions.");
            }
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
            auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
            size_t subsize = compute_total_size(subdims);
            for (const auto& e: val) {
                inspector<value_type>::serialize(e, subdims, m);
                m += subsize;
            }
        }
    }

    template <class It>
    static void unserialize(const It& vec_align, const std::vector<size_t>& dims, type& val) {
        std::vector<size_t> subdims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(subdims);
        for (size_t i = 0; i < dims[0]; ++i) {
            inspector<value_type>::unserialize(vec_align + i * subsize, subdims, val[i]);
        }
    }
};

}  // namespace details
}  // namespace HighFive
