#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <boost/numeric/ublas/matrix.hpp>

namespace HighFive {
namespace details {

template <typename T>
struct inspector<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;
    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& val) {
        return ndim + inspector<value_type>::getRank(val(0, 0));
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{val.size1(), val.size2()};
        auto s = inspector<value_type>::getDimensions(val(0, 0));
        sizes.insert(sizes.end(), s.begin(), s.end());
        return sizes;
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims.size() < ndim) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size() << " dimensions into a " << ndim
               << " boost::numeric::ublas::matrix";
            throw DataSpaceException(os.str());
        }
        val.resize(dims[0], dims[1], false);
    }

    static hdf5_type* data(type& val) {
        return inspector<value_type>::data(val(0, 0));
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(val(0, 0));
    }

    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* m) {
        size_t size = val.size1() * val.size2();
        auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(subdims);
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::serialize(*(&val(0, 0) + i), subdims, m + i * subsize);
        }
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        size_t size = val.size1() * val.size2();
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::unserialize(vec_align + i * subsize,
                                               next_dims,
                                               *(&val(0, 0) + i));
        }
    }
};

}  // namespace details
}  // namespace HighFive
