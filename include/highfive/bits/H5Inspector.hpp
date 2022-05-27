#include "../H5Reference.hpp"

namespace HighFive {
template <size_t N>
inline size_t compute_total_size(const std::array<size_t, N>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;

namespace details {
template <typename T>
struct inspector {
    using type = T;
    using base_type = unqualified_t<T>;
    using hdf5_type = base_type;

    static constexpr size_t ndim = 0;
    static constexpr size_t recursive_ndim = ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        return std::array<size_t, recursive_ndim>();
    }

    static std::vector<hdf5_type> serialize(const type& val) {
        return {val};
    }
};

template<>
struct inspector<std::string> {
    using type = std::string;
    using base_type = unqualified_t<type>;
    using hdf5_type = const char*;

    static constexpr size_t ndim = 0;
    static constexpr size_t recursive_ndim = ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        return std::array<size_t, recursive_ndim>();
    }

    static std::vector<hdf5_type> serialize(const type& val) {
        return {val.c_str()};
    }
};

template<>
struct inspector<Reference> {
    using type = Reference;
    using base_type = unqualified_t<type>;
    using hdf5_type = hobj_ref_t;

    static constexpr size_t ndim = 0;
    static constexpr size_t recursive_ndim = ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        return std::array<size_t, recursive_ndim>();
    }

    static std::vector<hdf5_type> serialize(const type& val) {
        hobj_ref_t ref;
        val.create_ref(&ref);
        return {ref};
    }
};

template <size_t N>
struct inspector<FixedLenStringArray<N>> {
    using type = FixedLenStringArray<N>;
    using base_type = FixedLenStringArray<N>;
    using hdf5_type = base_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        return std::array<size_t, recursive_ndim>{val.size()};
    }
};

template <typename T>
struct inspector<std::vector<T>> {
    using type = std::vector<T>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{val.size()};
        size_t index = ndim;
        if (!val.empty()) {
            for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
                sizes[index++] = s;
            }
        }
        return sizes;
    }

    static std::vector<hdf5_type> serialize(const type& val) {
        size_t size = compute_total_size(getDimensions(val));
        std::vector<hdf5_type> vec;
        vec.reserve(size);
        for (auto& e: val) {
            auto v = inspector<value_type>::serialize(e);
            vec.insert(vec.end(), v.begin(), v.end());
        }
        return vec;
    }
};

template <typename T>
struct inspector<T*> {
    using type = T*;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        throw std::string("Not possible to have size of a T*");
    }
};

template <typename T>
struct inspector<const T*>: public inspector<T*> {};


template <typename T, size_t N>
struct inspector<T[N]> {
    using type = T[N];
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{N};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }
};

template <typename T, size_t N>
struct inspector<std::array<T, N>> {
    using type = std::array<T, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{N};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }
};

#ifdef H5_USE_EIGEN
template <typename T, int M, int N>
struct inspector<Eigen::Matrix<T, M, N>> {
    using type = Eigen::Matrix<T, M, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{static_cast<size_t>(val.rows()),
                                                 static_cast<size_t>(val.cols())};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val.data()[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }
};
#endif

#ifdef H5_USE_BOOST
template <typename T, size_t Dims>
struct inspector<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;

    static constexpr size_t ndim = Dims;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes;
        for (size_t i = 0; i < ndim; ++i) {
            sizes[i] = val.shape()[i];
        }

        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val.data()[0])) {
            sizes[index++] = s;
        }
        return sizes;
    }
};

template <typename T>
struct inspector<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;

    static constexpr size_t ndim = 2;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{val.size1(), val.size2()};
        size_t index = ndim;
        for (const auto& s: inspector<value_type>::getDimensions(val(0, 0))) {
            sizes[index++] = s;
        }
        return sizes;
    }
};
#endif
}
}
