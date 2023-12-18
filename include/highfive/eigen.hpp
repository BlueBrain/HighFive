#pragma once
#ifdef H5_USE_EIGEN

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <Eigen/Eigen>


namespace HighFive {
namespace details {

template <typename T, int M, int N>
struct inspector<Eigen::Matrix<T, M, N>> {
    using type = Eigen::Matrix<T, M, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;


    static void assert_not_buggy(Eigen::Index nrows, Eigen::Index ncols) {
        if (nrows > 1 && ncols > 1) {
            throw std::runtime_error(
                "HighFive has been broken for Eigen::Matrix. Please check "
                "https://github.com/BlueBrain/HighFive/issues/532.");
        }
    }

    static std::vector<size_t> getDimensions(const type& val) {
        assert_not_buggy(val.rows(), val.cols());

        std::vector<size_t> sizes{static_cast<size_t>(val.rows()), static_cast<size_t>(val.cols())};
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
        if (dims[0] != static_cast<size_t>(val.rows()) ||
            dims[1] != static_cast<size_t>(val.cols())) {
            val.resize(static_cast<typename type::Index>(dims[0]),
                       static_cast<typename type::Index>(dims[1]));
        }

        assert_not_buggy(val.rows(), val.cols());
    }

    static hdf5_type* data(type& val) {
        assert_not_buggy(val.rows(), val.cols());
        return inspector<value_type>::data(*val.data());
    }

    static const hdf5_type* data(const type& val) {
        assert_not_buggy(val.rows(), val.cols());
        return inspector<value_type>::data(*val.data());
    }

    static void serialize(const type& val, hdf5_type* m) {
        assert_not_buggy(val.rows(), val.cols());
        std::memcpy(m, val.data(), static_cast<size_t>(val.size()) * sizeof(hdf5_type));
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        assert_not_buggy(val.rows(), val.cols());
        if (dims.size() < 2) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size()
               << " dimensions into an eigen-matrix.";
            throw DataSpaceException(os.str());
        }
        std::memcpy(val.data(), vec_align, compute_total_size(dims) * sizeof(hdf5_type));
    }
};

}  // namespace details
}  // namespace HighFive

#endif
