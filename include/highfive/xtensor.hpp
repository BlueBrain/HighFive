#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <xtensor/xtensor.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xadapt.hpp>

namespace HighFive {
namespace details {

template <class XTensor>
struct xtensor_get_rank;

template <typename T, size_t N, xt::layout_type L>
struct xtensor_get_rank<xt::xtensor<T, N, L>> {
    static constexpr size_t value = N;
};

template <class EC, size_t N, xt::layout_type L, class Tag>
struct xtensor_get_rank<xt::xtensor_adaptor<EC, N, L, Tag>> {
    static constexpr size_t value = N;
};

template <class Derived, class XTensorType, xt::layout_type L>
struct xtensor_inspector_base {
    using type = XTensorType;
    using value_type = typename type::value_type;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;

    static_assert(std::is_same<value_type, base_type>::value,
                  "HighFive's XTensor support only works for scalar elements.");

    static constexpr bool IsConstExprRowMajor = L == xt::layout_type::row_major;
    static constexpr bool is_trivially_copyable = IsConstExprRowMajor &&
                                                  std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_copyable;

    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& val) {
        // Non-scalar elements are not supported.
        return val.shape().size();
    }

    static const value_type& getAnyElement(const type& val) {
        return val.unchecked(0);
    }

    static value_type& getAnyElement(type& val) {
        return val.unchecked(0);
    }

    static std::vector<size_t> getDimensions(const type& val) {
        auto shape = val.shape();
        return {shape.begin(), shape.end()};
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(Derived::shapeFromDims(dims));
    }

    static hdf5_type* data(type& val) {
        if (!is_trivially_copyable) {
            throw DataSetException("Invalid used of `inspector<XTensor>::data`.");
        }

        if (val.size() == 0) {
            return nullptr;
        }

        return inspector<value_type>::data(getAnyElement(val));
    }

    static const hdf5_type* data(const type& val) {
        if (!is_trivially_copyable) {
            throw DataSetException("Invalid used of `inspector<XTensor>::data`.");
        }

        if (val.size() == 0) {
            return nullptr;
        }

        return inspector<value_type>::data(getAnyElement(val));
    }

    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* m) {
        // since we only support scalar types we know all dims belong to us.
        size_t size = compute_total_size(dims);
        xt::adapt(m, size, xt::no_ownership(), dims) = val;
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        // since we only support scalar types we know all dims belong to us.
        size_t size = compute_total_size(dims);
        val = xt::adapt(vec_align, size, xt::no_ownership(), dims);
    }
};

template <class XTensorType, xt::layout_type L>
struct xtensor_inspector
    : public xtensor_inspector_base<xtensor_inspector<XTensorType, L>, XTensorType, L> {
  private:
    using super = xtensor_inspector_base<xtensor_inspector<XTensorType, L>, XTensorType, L>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;

    static constexpr size_t ndim = xtensor_get_rank<XTensorType>::value;
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static std::array<size_t, ndim> shapeFromDims(const std::vector<size_t>& dims) {
        std::array<size_t, ndim> shape;
        std::copy(dims.cbegin(), dims.cend(), shape.begin());
        return shape;
    }
};

template <class XArrayType, xt::layout_type L>
struct xarray_inspector
    : public xtensor_inspector_base<xarray_inspector<XArrayType, L>, XArrayType, L> {
  private:
    using super = xtensor_inspector_base<xarray_inspector<XArrayType, L>, XArrayType, L>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;

    static constexpr size_t min_ndim = 0 + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = 1024 + inspector<value_type>::max_ndim;

    static const std::vector<size_t>& shapeFromDims(const std::vector<size_t>& dims) {
        return dims;
    }
};

template <typename T, size_t N, xt::layout_type L>
struct inspector<xt::xtensor<T, N, L>>: public xtensor_inspector<xt::xtensor<T, N, L>, L> {
  private:
    using super = xtensor_inspector<xt::xtensor<T, N, L>, L>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

template <typename T, xt::layout_type L>
struct inspector<xt::xarray<T, L>>: public xarray_inspector<xt::xarray<T, L>, L> {
  private:
    using super = xarray_inspector<xt::xarray<T, L>, L>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

template <typename CT, class... S>
struct inspector<xt::xview<CT, S...>>
    : public xarray_inspector<xt::xview<CT, S...>, xt::layout_type::any> {
  private:
    using super = xarray_inspector<xt::xview<CT, S...>, xt::layout_type::any>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};


template <class EC, xt::layout_type L, class SC, class Tag>
struct inspector<xt::xarray_adaptor<EC, L, SC, Tag>>
    : public xarray_inspector<xt::xarray_adaptor<EC, L, SC, Tag>, xt::layout_type::any> {
  private:
    using super = xarray_inspector<xt::xarray_adaptor<EC, L, SC, Tag>, xt::layout_type::any>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

template <class EC, size_t N, xt::layout_type L, class Tag>
struct inspector<xt::xtensor_adaptor<EC, N, L, Tag>>
    : public xtensor_inspector<xt::xtensor_adaptor<EC, N, L, Tag>, xt::layout_type::any> {
  private:
    using super = xtensor_inspector<xt::xtensor_adaptor<EC, N, L, Tag>, xt::layout_type::any>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

}  // namespace details
}  // namespace HighFive
