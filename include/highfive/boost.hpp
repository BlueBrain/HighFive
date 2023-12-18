#pragma once
#ifdef H5_USE_BOOST

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <boost/multi_array.hpp>
// starting Boost 1.64, serialization header must come before ublas
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace HighFive {
namespace details {

template <typename T, size_t Dims>
struct inspector<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = Dims;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes;
        for (size_t i = 0; i < ndim; ++i) {
            sizes.push_back(val.shape()[i]);
        }
        auto s = inspector<value_type>::getDimensions(val.data()[0]);
        sizes.insert(sizes.end(), s.begin(), s.end());
        return sizes;
    }

    static size_t getSizeVal(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        return compute_total_size(dims);
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

    static hdf5_type* data(type& val) {
        return inspector<value_type>::data(*val.data());
    }

    static const hdf5_type* data(const type& val) {
        return inspector<value_type>::data(*val.data());
    }

    template <class It>
    static void serialize(const type& val, It m) {
        size_t size = val.num_elements();
        size_t subsize = inspector<value_type>::getSizeVal(*val.origin());
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::serialize(*(val.origin() + i), m + i * subsize);
        }
    }

    template <class It>
    static void unserialize(It vec_align, const std::vector<size_t>& dims, type& val) {
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        for (size_t i = 0; i < val.num_elements(); ++i) {
            inspector<value_type>::unserialize(vec_align + i * subsize,
                                               next_dims,
                                               *(val.origin() + i));
        }
    }
};

template <typename T>
struct inspector<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = unqualified_t<T>;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{val.size1(), val.size2()};
        auto s = inspector<value_type>::getDimensions(val(0, 0));
        sizes.insert(sizes.end(), s.begin(), s.end());
        return sizes;
    }

    static size_t getSizeVal(const type& val) {
        return compute_total_size(getDimensions(val));
    }

    static size_t getSize(const std::vector<size_t>& dims) {
        return compute_total_size(dims);
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

    static void serialize(const type& val, hdf5_type* m) {
        size_t size = val.size1() * val.size2();
        size_t subsize = inspector<value_type>::getSizeVal(val(0, 0));
        for (size_t i = 0; i < size; ++i) {
            inspector<value_type>::serialize(*(&val(0, 0) + i), m + i * subsize);
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

#endif
