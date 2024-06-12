#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>

namespace HighFive {
namespace details {

template <class EigenType>
struct eigen_inspector {
    using type = EigenType;
    using value_type = typename EigenType::Scalar;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;


    static_assert(int(EigenType::ColsAtCompileTime) == int(EigenType::MaxColsAtCompileTime),
                  "Padding isn't supported.");
    static_assert(int(EigenType::RowsAtCompileTime) == int(EigenType::MaxRowsAtCompileTime),
                  "Padding isn't supported.");

    static constexpr bool is_row_major() {
        return EigenType::ColsAtCompileTime == 1 || EigenType::RowsAtCompileTime == 1 ||
               EigenType::IsRowMajor;
    }


    static constexpr size_t ndim = 2;
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;
    static constexpr bool is_trivially_copyable = is_row_major() &&
                                                  std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& val) {
        return ndim + inspector<value_type>::getRank(val.data()[0]);
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes{static_cast<size_t>(val.rows()), static_cast<size_t>(val.cols())};
        auto s = inspector<value_type>::getDimensions(val.data()[0]);
        sizes.insert(sizes.end(), s.begin(), s.end());
        return sizes;
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims[0] != static_cast<size_t>(val.rows()) ||
            dims[1] != static_cast<size_t>(val.cols())) {
            val.resize(static_cast<typename type::Index>(dims[0]),
                       static_cast<typename type::Index>(dims[1]));
        }
    }

    static hdf5_type* data(type& val) {
        if (!is_trivially_copyable) {
            throw DataSetException("Invalid used of `inspector<Eigen::Matrix<...>>::data`.");
        }

        return inspector<value_type>::data(*val.data());
    }

    static const hdf5_type* data(const type& val) {
        if (!is_trivially_copyable) {
            throw DataSetException("Invalid used of `inspector<Eigen::Matrix<...>>::data`.");
        }

        return inspector<value_type>::data(*val.data());
    }

    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* m) {
        Eigen::Index n_rows = val.rows();
        Eigen::Index n_cols = val.cols();

        auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
        auto subsize = compute_total_size(subdims);
        for (Eigen::Index i = 0; i < n_rows; ++i) {
            for (Eigen::Index j = 0; j < n_cols; ++j) {
                inspector<value_type>::serialize(val(i, j), dims, m);
                m += subsize;
            }
        }
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        if (dims.size() < 2) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size()
               << " dimensions into an eigen-matrix.";
            throw DataSpaceException(os.str());
        }

        auto n_rows = static_cast<Eigen::Index>(dims[0]);
        auto n_cols = static_cast<Eigen::Index>(dims[1]);

        auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
        auto subsize = compute_total_size(subdims);
        for (Eigen::Index i = 0; i < n_rows; ++i) {
            for (Eigen::Index j = 0; j < n_cols; ++j) {
                inspector<value_type>::unserialize(vec_align, subdims, val(i, j));
                vec_align += subsize;
            }
        }
    }
};

template <typename T, int M, int N, int Options>
struct inspector<Eigen::Matrix<T, M, N, Options>>
    : public eigen_inspector<Eigen::Matrix<T, M, N, Options>> {
  private:
    using super = eigen_inspector<Eigen::Matrix<T, M, N, Options>>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

template <typename T, int M, int N, int Options>
struct inspector<Eigen::Array<T, M, N, Options>>
    : public eigen_inspector<Eigen::Array<T, M, N, Options>> {
  private:
    using super = eigen_inspector<Eigen::Array<T, M, N, Options>>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};


template <typename PlainObjectType, int MapOptions>
struct inspector<Eigen::Map<PlainObjectType, MapOptions>>
    : public eigen_inspector<Eigen::Map<PlainObjectType, MapOptions>> {
  private:
    using super = eigen_inspector<Eigen::Map<PlainObjectType, MapOptions>>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims[0] != static_cast<size_t>(val.rows()) ||
            dims[1] != static_cast<size_t>(val.cols())) {
            throw DataSetException("Eigen::Map has invalid shape and can't be resized.");
        }
    }
};


}  // namespace details
}  // namespace HighFive
