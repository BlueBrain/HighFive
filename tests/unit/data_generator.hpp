#pragma once

#include <limits>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <array>

#include <highfive/bits/H5Inspector_misc.hpp>

#ifdef HIGHFIVE_TEST_BOOST
#include <highfive/boost.hpp>
#endif

#ifdef HIGHFIVE_TEST_BOOST_SPAN
#include <highfive/boost_span.hpp>
#endif

#ifdef HIGHFIVE_TEST_EIGEN
#include <highfive/eigen.hpp>
#endif

#ifdef HIGHFIVE_TEST_SPAN
#include <highfive/span.hpp>
#endif

#ifdef HIGHFIVE_TEST_XTENSOR
#include <highfive/xtensor.hpp>
#endif


namespace HighFive {
namespace testing {

template <class Dims>
std::vector<size_t> lstrip(const Dims& indices, size_t n) {
    std::vector<size_t> subindices(indices.size() - n);
    for (size_t i = 0; i < subindices.size(); ++i) {
        subindices[i] = indices[i + n];
    }

    return subindices;
}

template <class Dims>
size_t ravel(std::vector<size_t>& indices, const Dims& dims) {
    size_t rank = dims.size();
    size_t linear_index = 0;
    size_t ld = 1;
    for (size_t kk = 0; kk < rank; ++kk) {
        auto k = rank - 1 - kk;
        linear_index += indices[k] * ld;
        ld *= dims[k];
    }

    return linear_index;
}

template <class Dims>
std::vector<size_t> unravel(size_t flat_index, const Dims& dims) {
    size_t rank = dims.size();
    size_t ld = 1;
    std::vector<size_t> indices(rank);
    for (size_t kk = 0; kk < rank; ++kk) {
        auto k = rank - 1 - kk;
        indices[k] = (flat_index / ld) % dims[k];
        ld *= dims[k];
    }

    return indices;
}

template <class Dims>
size_t flat_size(const Dims& dims) {
    size_t n = 1;
    for (auto d: dims) {
        n *= d;
    }

    return n;
}

template <class Container, class = void>
struct ContainerTraits;

// -- Scalar basecases ---------------------------------------------------------
template <class T>
struct ScalarContainerTraits {
    using container_type = T;
    using base_type = T;

    static constexpr bool is_view = false;
    static constexpr size_t rank = 0;

    static void set(container_type& array, std::vector<size_t> /* indices */, base_type value) {
        array = value;
    }

    static const base_type& get(const container_type& array, std::vector<size_t> /* indices */) {
        return array;
    }

    static void assign(container_type& dst, const container_type& src) {
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& /* dims */) {
        return container_type{};
    }

    static void deallocate(container_type& /* array */, const std::vector<size_t>& /* dims */) {}

    static void sanitize_dims(std::vector<size_t>& /* dims */, size_t /* axis */) {}
};

template <class T>
struct ContainerTraits<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
    : public ScalarContainerTraits<T> {};

template <class T>
struct ContainerTraits<T, typename std::enable_if<std::is_integral<T>::value>::type>
    : public ScalarContainerTraits<T> {};

template <>
struct ContainerTraits<std::string>: public ScalarContainerTraits<std::string> {};

// -- STL ----------------------------------------------------------------------
template <>
struct ContainerTraits<std::vector<bool>> {
    using container_type = std::vector<bool>;
    using value_type = bool;
    using base_type = bool;

    static constexpr bool is_view = false;
    static constexpr size_t rank = 1;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        array[indices[0]] = value;
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        return array[indices[0]];
    }

    static void assign(container_type& dst, const container_type& src) {
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& dims) {
        container_type array(dims[0]);
        return array;
    }

    static void deallocate(container_type& /* array */, const std::vector<size_t>& /* dims */) {}

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        ContainerTraits<value_type>::sanitize_dims(dims, axis + 1);
    }
};

template <class Container, class ValueType = typename Container::value_type>
struct STLLikeContainerTraits {
    using container_type = Container;
    using value_type = ValueType;
    using base_type = typename ContainerTraits<value_type>::base_type;

    static constexpr bool is_view = ContainerTraits<value_type>::is_view;
    static constexpr size_t rank = 1 + ContainerTraits<value_type>::rank;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        return ContainerTraits<value_type>::set(array[indices[0]], lstrip(indices, 1), value);
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        return ContainerTraits<value_type>::get(array[indices[0]], lstrip(indices, 1));
    }

    static void assign(container_type& dst, const container_type& src) {
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& dims) {
        container_type array;
        array.reserve(dims[0]);
        for (size_t i = 0; i < dims[0]; ++i) {
            auto value = ContainerTraits<value_type>::allocate(lstrip(dims, 1));
            array.push_back(value);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        for (size_t i = 0; i < dims[0]; ++i) {
            ContainerTraits<value_type>::deallocate(array[i], lstrip(dims, 1));
        }
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        ContainerTraits<value_type>::sanitize_dims(dims, axis + 1);
    }
};

template <class T>
struct ContainerTraits<std::vector<T>>: public STLLikeContainerTraits<std::vector<T>> {};

template <class T, size_t N>
struct ContainerTraits<std::array<T, N>>: public STLLikeContainerTraits<std::array<T, N>> {
  private:
    using super = STLLikeContainerTraits<std::array<T, N>>;

  public:
    using container_type = typename super::container_type;
    using base_type = typename super::base_type;
    using value_type = typename super::value_type;

  public:
    static container_type allocate(const std::vector<size_t>& dims) {
        if (N != dims[0]) {
            throw std::runtime_error("broken logic: static and runtime size don't match.");
        }

        container_type array;
        for (size_t i = 0; i < dims[0]; ++i) {
            auto value = ContainerTraits<value_type>::allocate(lstrip(dims, 1));
            ContainerTraits<value_type>::assign(array[i], value);
        }

        return array;
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        dims[axis] = N;
        ContainerTraits<value_type>::sanitize_dims(dims, axis + 1);
    }
};

// Anything with the same API as `std::span` can implemented by inheriting from
// this class.
//
// The template parameter `DynamicExtent` is the equivalent of the magic number
// `std::dynamic_extent`.
template <class Span, size_t DynamicExtent>
struct STLSpanLikeContainerTraits: public STLLikeContainerTraits<Span> {
  private:
    using super = STLLikeContainerTraits<Span>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;

    static constexpr bool is_view = true;

    static container_type allocate(const std::vector<size_t>& dims) {
        size_t n_elements = dims[0];
        value_type* ptr = new value_type[n_elements];

        container_type array = container_type(ptr, n_elements);

        for (size_t i = 0; i < n_elements; ++i) {
            auto element = ContainerTraits<value_type>::allocate(lstrip(dims, 1));
            ContainerTraits<value_type>::assign(array[i], element);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        size_t n_elements = dims[0];
        for (size_t i = 0; i < n_elements; ++i) {
            ContainerTraits<value_type>::deallocate(array[i], lstrip(dims, 1));
        }

        delete[] array.data();
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        if (Span::extent != DynamicExtent) {
            dims[axis] = Span::extent;
            ContainerTraits<value_type>::sanitize_dims(dims, axis + 1);
        }
    }
};


#ifdef HIGHFIVE_TEST_SPAN
template <class T, std::size_t Extent>
struct ContainerTraits<std::span<T, Extent>>
    : public STLSpanLikeContainerTraits<std::span<T, Extent>, std::dynamic_extent> {
  private:
    using super = STLSpanLikeContainerTraits<std::span<T, Extent>, std::dynamic_extent>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
};
#endif


template <class T, size_t n>
struct ContainerTraits<T[n]> {
    using container_type = T[n];
    using value_type = T;
    using base_type = typename ContainerTraits<value_type>::base_type;

    static constexpr bool is_view = ContainerTraits<value_type>::is_view;
    static constexpr size_t rank = 1 + ContainerTraits<value_type>::rank;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        return ContainerTraits<value_type>::set(array[indices[0]], lstrip(indices, 1), value);
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        return ContainerTraits<value_type>::get(array[indices[0]], lstrip(indices, 1));
    }

    static void assign(container_type& dst, const container_type& src) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i];
        }
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        dims[axis] = n;
        ContainerTraits<value_type>::sanitize_dims(dims, axis + 1);
    }
};


// -- Boost  -------------------------------------------------------------------
#ifdef HIGHFIVE_TEST_BOOST
template <class T, size_t n>
struct ContainerTraits<boost::multi_array<T, n>> {
    using container_type = typename boost::multi_array<T, n>;
    using value_type = T;
    using base_type = typename ContainerTraits<value_type>::base_type;

    static constexpr bool is_view = ContainerTraits<value_type>::is_view;
    static constexpr size_t rank = n + ContainerTraits<value_type>::rank;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        auto i = std::vector<size_t>(indices.begin(), indices.begin() + n);
        return ContainerTraits<value_type>::set(array(i), lstrip(indices, n), value);
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        auto i = std::vector<size_t>(indices.begin(), indices.begin() + n);
        return ContainerTraits<value_type>::get(array(i), lstrip(indices, n));
    }

    static void assign(container_type& dst, const container_type& src) {
        auto const* const shape = src.shape();
        dst.resize(std::vector<size_t>(shape, shape + n));
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + n);
        container_type array(local_dims);

        size_t n_elements = flat_size(local_dims);
        for (size_t i = 0; i < n_elements; ++i) {
            auto element = ContainerTraits<value_type>::allocate(lstrip(dims, n));
            set(array, unravel(i, local_dims), element);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + n);
        size_t n_elements = flat_size(local_dims);
        for (size_t i = 0; i < n_elements; ++i) {
            ContainerTraits<value_type>::deallocate(array(unravel(i, local_dims)), lstrip(dims, n));
        }
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        ContainerTraits<value_type>::sanitize_dims(dims, axis + n);
    }
};

template <class T>
struct ContainerTraits<boost::numeric::ublas::matrix<T>> {
    using container_type = typename boost::numeric::ublas::matrix<T>;
    using value_type = T;
    using base_type = typename ContainerTraits<value_type>::base_type;

    static constexpr bool is_view = ContainerTraits<value_type>::is_view;
    static constexpr size_t rank = 2 + ContainerTraits<value_type>::rank;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        auto i = indices[0];
        auto j = indices[1];
        return ContainerTraits<value_type>::set(array(i, j), lstrip(indices, 2), value);
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        auto i = indices[0];
        auto j = indices[1];
        return ContainerTraits<value_type>::get(array(i, j), lstrip(indices, 2));
    }

    static void assign(container_type& dst, const container_type& src) {
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + 2);
        container_type array(local_dims[0], local_dims[1]);

        size_t n_elements = flat_size(local_dims);
        for (size_t i = 0; i < n_elements; ++i) {
            auto indices = unravel(i, local_dims);
            auto element = ContainerTraits<value_type>::allocate(lstrip(dims, 2));

            ContainerTraits<value_type>::assign(array(indices[0], indices[1]), element);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + 2);
        size_t n_elements = flat_size(local_dims);
        for (size_t i = 0; i < n_elements; ++i) {
            auto indices = unravel(i, local_dims);
            ContainerTraits<value_type>::deallocate(array(indices[0], indices[1]), lstrip(dims, 2));
        }
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        ContainerTraits<value_type>::sanitize_dims(dims, axis + 2);
    }
};

#endif

#if HIGHFIVE_TEST_BOOST_SPAN
template <class T, std::size_t Extent>
struct ContainerTraits<boost::span<T, Extent>>
    : public STLSpanLikeContainerTraits<boost::span<T, Extent>, boost::dynamic_extent> {
  private:
    using super = STLSpanLikeContainerTraits<boost::span<T, Extent>, boost::dynamic_extent>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
};
#endif

// -- Eigen  -------------------------------------------------------------------
#if HIGHFIVE_TEST_EIGEN

template <typename EigenType>
struct EigenContainerTraits {
    using container_type = EigenType;
    using value_type = typename EigenType::Scalar;
    using base_type = typename ContainerTraits<value_type>::base_type;

    static constexpr bool is_view = ContainerTraits<value_type>::is_view;
    static constexpr size_t rank = 2 + ContainerTraits<value_type>::rank;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        auto i = static_cast<Eigen::Index>(indices[0]);
        auto j = static_cast<Eigen::Index>(indices[1]);
        return ContainerTraits<value_type>::set(array(i, j), lstrip(indices, 2), value);
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        auto i = static_cast<Eigen::Index>(indices[0]);
        auto j = static_cast<Eigen::Index>(indices[1]);
        return ContainerTraits<value_type>::get(array(i, j), lstrip(indices, 2));
    }

    static void assign(container_type& dst, const container_type& src) {
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + 2);
        auto n_rows = static_cast<Eigen::Index>(local_dims[0]);
        auto n_cols = static_cast<Eigen::Index>(local_dims[1]);
        container_type array = container_type::Zero(n_rows, n_cols);

        size_t n_elements = flat_size(local_dims);
        for (size_t i = 0; i < n_elements; ++i) {
            auto element = ContainerTraits<value_type>::allocate(lstrip(dims, 2));
            set(array, unravel(i, local_dims), element);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + 2);
        size_t n_elements = flat_size(local_dims);
        for (size_t i_flat = 0; i_flat < n_elements; ++i_flat) {
            auto indices = unravel(i_flat, local_dims);
            auto i = static_cast<Eigen::Index>(indices[0]);
            auto j = static_cast<Eigen::Index>(indices[1]);
            ContainerTraits<value_type>::deallocate(array(i, j), lstrip(dims, 2));
        }
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        if (EigenType::RowsAtCompileTime != Eigen::Dynamic) {
            dims[axis + 0] = static_cast<size_t>(EigenType::RowsAtCompileTime);
        }

        if (EigenType::ColsAtCompileTime != Eigen::Dynamic) {
            dims[axis + 1] = static_cast<size_t>(EigenType::ColsAtCompileTime);
        }
        ContainerTraits<value_type>::sanitize_dims(dims, axis + 2);
    }
};

template <class T, int N_ROWS, int N_COLS, int Options>
struct ContainerTraits<Eigen::Matrix<T, N_ROWS, N_COLS, Options>>
    : public EigenContainerTraits<Eigen::Matrix<T, N_ROWS, N_COLS, Options>> {
  private:
    using super = EigenContainerTraits<Eigen::Matrix<T, N_ROWS, N_COLS, Options>>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
};

template <class T, int N_ROWS, int N_COLS, int Options>
struct ContainerTraits<Eigen::Array<T, N_ROWS, N_COLS, Options>>
    : public EigenContainerTraits<Eigen::Array<T, N_ROWS, N_COLS, Options>> {
  private:
    using super = EigenContainerTraits<Eigen::Array<T, N_ROWS, N_COLS, Options>>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
};

template <class PlainObjectType, int MapOptions>
struct ContainerTraits<Eigen::Map<PlainObjectType, MapOptions>>
    : public EigenContainerTraits<Eigen::Map<PlainObjectType, MapOptions>> {
  private:
    using super = EigenContainerTraits<Eigen::Map<PlainObjectType, MapOptions>>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;

    static constexpr bool is_view = true;

    static container_type allocate(const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + 2);
        auto n_rows = static_cast<Eigen::Index>(local_dims[0]);
        auto n_cols = static_cast<Eigen::Index>(local_dims[1]);

        size_t n_elements = flat_size(local_dims);
        value_type* ptr = new value_type[n_elements];

        container_type array = container_type(ptr, n_rows, n_cols);

        for (size_t i = 0; i < n_elements; ++i) {
            auto element = ContainerTraits<value_type>::allocate(lstrip(dims, 2));
            ContainerTraits::set(array, unravel(i, local_dims), element);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + 2);
        size_t n_elements = flat_size(local_dims);
        for (size_t i_flat = 0; i_flat < n_elements; ++i_flat) {
            auto indices = unravel(i_flat, local_dims);
            auto i = static_cast<Eigen::Index>(indices[0]);
            auto j = static_cast<Eigen::Index>(indices[1]);
            ContainerTraits<value_type>::deallocate(array(i, j), lstrip(dims, 2));
        }

        delete[] array.data();
    }
};


#endif

// -- XTensor  -----------------------------------------------------------------

#if HIGHFIVE_TEST_XTENSOR
template <typename XTensorType, size_t Rank>
struct XTensorContainerTraits {
    using container_type = XTensorType;
    using value_type = typename container_type::value_type;
    using base_type = typename ContainerTraits<value_type>::base_type;

    static constexpr size_t rank = Rank;
    static constexpr bool is_view = ContainerTraits<value_type>::is_view;

    static void set(container_type& array,
                    const std::vector<size_t>& indices,
                    const base_type& value) {
        std::vector<size_t> local_indices(indices.begin(), indices.begin() + rank);
        return ContainerTraits<value_type>::set(array[local_indices], lstrip(indices, rank), value);
    }

    static base_type get(const container_type& array, const std::vector<size_t>& indices) {
        std::vector<size_t> local_indices(indices.begin(), indices.begin() + rank);
        return ContainerTraits<value_type>::get(array[local_indices], lstrip(indices, rank));
    }

    static void assign(container_type& dst, const container_type& src) {
        dst = src;
    }

    static container_type allocate(const std::vector<size_t>& dims) {
        const auto& local_dims = details::inspector<XTensorType>::shapeFromDims(dims);
        auto array = container_type(local_dims);

        size_t n_elements = flat_size(local_dims);
        for (size_t i = 0; i < n_elements; ++i) {
            auto element = ContainerTraits<value_type>::allocate(lstrip(dims, rank));
            set(array, unravel(i, local_dims), element);
        }

        return array;
    }

    static void deallocate(container_type& array, const std::vector<size_t>& dims) {
        auto local_dims = std::vector<size_t>(dims.begin(), dims.begin() + rank);
        size_t n_elements = flat_size(local_dims);
        for (size_t i_flat = 0; i_flat < n_elements; ++i_flat) {
            auto indices = unravel(i_flat, local_dims);
            std::vector<size_t> local_indices(indices.begin(), indices.begin() + rank);
            ContainerTraits<value_type>::deallocate(array[local_indices], lstrip(dims, rank));
        }
    }

    static void sanitize_dims(std::vector<size_t>& dims, size_t axis) {
        ContainerTraits<value_type>::sanitize_dims(dims, axis + rank);
    }
};

template <class T, size_t rank, xt::layout_type layout>
struct ContainerTraits<xt::xtensor<T, rank, layout>>
    : public XTensorContainerTraits<xt::xtensor<T, rank, layout>, rank> {
  private:
    using super = XTensorContainerTraits<xt::xtensor<T, rank, layout>, rank>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
};

template <class T, xt::layout_type layout>
struct ContainerTraits<xt::xarray<T, layout>>
    : public XTensorContainerTraits<xt::xarray<T, layout>, 2> {
  private:
    using super = XTensorContainerTraits<xt::xarray<T, layout>, 2>;

  public:
    using container_type = typename super::container_type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
};

#endif

template <class T, class C>
T copy(const C& src, const std::vector<size_t>& dims) {
    auto dst = ContainerTraits<T>::allocate(dims);
    for (size_t i = 0; i < flat_size(dims); ++i) {
        auto indices = unravel(i, dims);
        ContainerTraits<T>::set(dst, indices, ContainerTraits<C>::get(src, indices));
    }

    return dst;
}

template <class T>
T default_real_value(const std::vector<size_t>& indices, T shift, T base, T factor) {
    auto value = T(0);

    auto isum = std::accumulate(indices.begin(), indices.end(), size_t(0));
    auto sign = (std::is_signed<T>::value) && (isum % 2 == 1) ? T(-1) : T(1);

    for (size_t k = 0; k < indices.size(); ++k) {
        value += T(indices[k]) * T(std::pow(shift, T(k))) * base;
    }

    return sign * value * factor;
}

std::vector<std::string> ascii_alphabet = {"a", "b", "c", "d", "e", "f"};

std::string default_string(size_t offset, size_t length, const std::vector<std::string>& alphabet) {
    std::string s = "";
    for (size_t k = 0; k < length; ++k) {
        s += alphabet[(offset + k) % alphabet.size()];
    }

    return s;
}

std::string default_fixed_length_ascii_string(const std::vector<size_t>& indices, size_t length) {
    auto isum = std::accumulate(indices.begin(), indices.end(), size_t(0));
    return default_string(isum, length, ascii_alphabet);
}

std::string default_variable_length_ascii_string(const std::vector<size_t>& indices) {
    auto isum = std::accumulate(indices.begin(), indices.end(), size_t(0));
    return default_string(isum, isum, ascii_alphabet);
}

template <class T, class = void>
struct DefaultValues;

template <class T>
struct DefaultValues<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    T operator()(const std::vector<size_t>& indices) const {
        auto eps = std::numeric_limits<T>::epsilon();
        return default_real_value(indices, T(100.0), T(0.01), T(1.0) + T(8) * eps);
    }
};

template <class T>
struct DefaultValues<T, typename std::enable_if<std::is_integral<T>::value>::type> {
    T operator()(const std::vector<size_t>& indices) const {
        return default_real_value(indices, T(100), T(1), T(1));
    }
};

template <>
struct DefaultValues<char> {
    char operator()(const std::vector<size_t>& indices) const {
        auto isum = std::accumulate(indices.begin(), indices.end(), size_t(0));
        return char(isum % size_t(std::numeric_limits<char>::max));
    }
};

template <>
struct DefaultValues<unsigned char> {
    unsigned char operator()(const std::vector<size_t>& indices) const {
        auto isum = std::accumulate(indices.begin(), indices.end(), size_t(0));
        return (unsigned char) (isum % size_t(std::numeric_limits<unsigned char>::max));
    }
};

template <>
struct DefaultValues<std::string> {
    std::string operator()(const std::vector<size_t>& indices) const {
        return default_variable_length_ascii_string(indices);
    }
};

template <>
struct DefaultValues<bool> {
    bool operator()(const std::vector<size_t>& indices) const {
        auto isum = std::accumulate(indices.begin(), indices.end(), size_t(0));
        return (isum % 2) == 0;
    }
};

template <class T, size_t N>
struct MultiDimVector {
    using type = std::vector<typename MultiDimVector<T, N - 1>::type>;
};

template <class T>
struct MultiDimVector<T, 0> {
    using type = T;
};

template <class C, class F>
void initialize_impl(C& array,
                     const std::vector<size_t>& dims,
                     std::vector<size_t>& indices,
                     size_t axis,
                     F f) {
    using traits = ContainerTraits<C>;
    if (axis == indices.size()) {
        auto value = f(indices);
        traits::set(array, indices, value);
    } else {
        for (size_t i = 0; i < dims[axis]; ++i) {
            indices[axis] = i;
            initialize_impl(array, dims, indices, axis + 1, f);
        }
    }
}

template <class C, class F>
void initialize(C& array, const std::vector<size_t>& dims, F f) {
    std::vector<size_t> indices(dims.size());
    initialize_impl(array, dims, indices, 0, f);
}

template <class C>
void initialize(C& array, const std::vector<size_t>& dims) {
    using traits = ContainerTraits<C>;
    initialize(array, dims, DefaultValues<typename traits::base_type>());
}


template <class Container>
class DataGenerator {
  public:
    using traits = ContainerTraits<Container>;
    using base_type = typename traits::base_type;
    using container_type = Container;

    constexpr static size_t rank = traits::rank;

  public:
    static container_type allocate(const std::vector<size_t>& dims) {
        return traits::allocate(dims);
    }

    template <class F>
    static container_type create(const std::vector<size_t>& dims, F f) {
        auto array = allocate(dims);
        initialize(array, dims, f);

        return array;
    }

    static container_type create(const std::vector<size_t>& dims) {
        return create(dims, DefaultValues<typename traits::base_type>());
    }

    static std::vector<size_t> default_dims() {
        using difference_type = std::vector<size_t>::difference_type;
        std::vector<size_t> oversized{2, 3, 5, 7, 2, 3, 5, 7};
        std::vector<size_t> dims(oversized.begin(), oversized.begin() + difference_type(rank));
        ContainerTraits<Container>::sanitize_dims(dims, /* axis = */ 0);

        return dims;
    }

    static void sanitize_dims(std::vector<size_t>& dims) {
        ContainerTraits<Container>::sanitize_dims(dims, /* axis = */ 0);
    }
};

}  // namespace testing
}  // namespace HighFive
