#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <boost/multi_array.hpp>

namespace HighFive {
namespace details {

template <typename T, size_t Dims>
struct inspector<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = Dims;
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = false;


    static size_t getRank(const type& val) {
        return ndim + inspector<value_type>::getRank(val.data()[0]);
    }

    static std::vector<size_t> getDimensions(const type& val) {
        auto rank = getRank(val);
        std::vector<size_t> sizes(rank, 1ul);
        for (size_t i = 0; i < ndim; ++i) {
            sizes[i] = val.shape()[i];
        }
        if (val.size() != 0) {
            auto s = inspector<value_type>::getDimensions(val.data()[0]);
            sizes.resize(ndim + s.size());
            for (size_t i = 0; i < s.size(); ++i) {
                sizes[ndim + i] = s[i];
            }
        }
        return sizes;
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims.size() < ndim) {
            std::ostringstream os;
            os << "Only '" << dims.size() << "' given but boost::multi_array is of size '" << ndim
               << "'.";
            throw DataSpaceException(os.str());
        }
        boost::array<typename type::index, Dims> ext;
        std::copy(dims.begin(), dims.begin() + ndim, ext.begin());
        val.resize(ext);
        std::vector<size_t> next_dims(dims.begin() + Dims, dims.end());
        std::size_t size = std::accumulate(dims.begin(),
                                           dims.begin() + Dims,
                                           std::size_t{1},
                                           std::multiplies<size_t>());
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::prepare(*(val.origin() + i), next_dims);
        }
    }

    static void assert_c_order(const type& val) {
        if (!(val.storage_order() == boost::c_storage_order())) {
            throw DataTypeException("Only C storage order is supported for 'boost::multi_array'.");
        }
    }

    static hdf5_type* data(type& val) {
        assert_c_order(val);
        return inspector<value_type>::data(*val.data());
    }

    static const hdf5_type* data(const type& val) {
        assert_c_order(val);
        return inspector<value_type>::data(*val.data());
    }

    template <class It>
    static void serialize(const type& val, const std::vector<size_t>& dims, It m) {
        assert_c_order(val);
        size_t size = val.num_elements();
        auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(subdims);
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::serialize(*(val.origin() + i), subdims, m + i * subsize);
        }
    }

    template <class It>
    static void unserialize(It vec_align, const std::vector<size_t>& dims, type& val) {
        assert_c_order(val);
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        for (size_t i = 0; i < val.num_elements(); ++i) {
            inspector<value_type>::unserialize(vec_align + i * subsize,
                                               next_dims,
                                               *(val.origin() + i));
        }
    }
};

}  // namespace details
}  // namespace HighFive
