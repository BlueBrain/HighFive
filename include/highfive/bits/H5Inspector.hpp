#include "../H5Reference.hpp"
#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
// starting Boost 1.64, serialization header must come before ublas
#include <boost/serialization/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif
#ifdef H5_USE_EIGEN
#include <Eigen/Eigen>
#endif

namespace HighFive {
inline size_t compute_total_size(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}
template <size_t N>
inline size_t compute_total_size(const std::array<size_t, N>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;

template <typename T>
class Writer {
  public:
    const T* get_pointer() {
      if (vec.empty()) {
        return ptr;
      } else {
        return vec.data();
      }
    }
    size_t get_size() {
      if (vec.empty()) {
        return size;
      } else {
        return vec.size();
      }
    }
    std::vector<T> vec{};
    size_t size{0};
    const T* ptr{nullptr};
};

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

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static type alloc(const std::vector<size_t>& /* dims */) {
        return type{};
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.vec = {val};
        return w;
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& /* dims */) {
        return vec[0];
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
        return {};
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static type alloc(const std::vector<size_t>& /* dims */) {
        return type{};
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.vec = {val.c_str()};
        return w;
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& /* dims */) {
        return std::string{vec[0]};
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

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static type alloc(const std::vector<size_t>& /* dims */) {
        return type{};
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        hobj_ref_t ref;
        val.create_ref(&ref);
        w.vec = {ref};
        return w;
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& /* dims */) {
        return Reference(vec[0]);
    }
};

template <size_t N>
struct inspector<FixedLenStringArray<N>> {
    using type = FixedLenStringArray<N>;
    using base_type = FixedLenStringArray<N>;
    using hdf5_type = char;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        return std::array<size_t, recursive_ndim>{val.size()};
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static type alloc(const std::vector<size_t>& /* dims */) {
        return type{};
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.vec.resize(N * compute_total_size(getDimensions(val)));
        for (size_t i = 0; i < val.size(); ++i) {
            memcpy(w.vec.data() + i * N, val[i], N);
        }
        return w;
    }

    static type unserialize(const hdf5_type* vec, const std::vector<size_t>& dims) {
        type val = alloc(dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            std::array<char, N> s;
            memcpy(s.data(), vec+(i*N), N);
            val.push_back(s);
        }
        return val;
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

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(dims[0]);
    }

    static type alloc(const std::vector<size_t>& dims) {
        type val;
        prepare(val, dims);
        return val;
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        size_t size = compute_total_size(getDimensions(val));
        w.vec.reserve(size);
        for (auto& e: val) {
            auto v = inspector<value_type>::serialize(e);
            w.vec.insert(w.vec.end(), v.get_pointer(), v.get_pointer() + v.get_size());
        }
        
        return w;
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        type val = alloc(dims);
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        size_t next_size = compute_total_size(next_dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            val[i] = inspector<value_type>::unserialize(vec_align + i * next_size, next_dims);
        }
        return val;
    }
};


template <typename T, size_t N>
struct inspector<std::array<T, N>> {
    using type = std::array<T, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& val) {
        std::array<size_t, recursive_ndim> sizes{N};
        size_t index = ndim;
        if (!val.empty()) {
            for (const auto& s: inspector<value_type>::getDimensions(val[0])) {
                sizes[index++] = s;
            }
        }
        return sizes;
    }

    static void prepare(type& /* val */, const std::vector<size_t>& /* dims */) {}

    static type alloc(const std::vector<size_t>& /* dims */) {
        return type{};
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        size_t size = compute_total_size(getDimensions(val));
        w.vec.reserve(size);
        for (auto& e: val) {
            auto v = inspector<value_type>::serialize(e);
            w.vec.insert(w.vec.end(), v.get_pointer(), v.get_pointer() + v.get_size());
        }
        return w;
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims[0] != N) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims[0] << " elements into an array with "
               << N << " elements.";
            throw DataSpaceException(os.str());
        }
        type val = alloc(dims);
        std::vector<size_t> next_dims(dims.begin() + 1, dims.end());
        size_t next_size = compute_total_size(next_dims);
        for (size_t i = 0; i < dims[0]; ++i) {
            val[i] = inspector<value_type>::unserialize(vec_align + i * next_size, next_dims);
        }
        return val;
    }
};

template <typename T>
struct inspector<T*> {
    using type = T*;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        throw std::string("Not possible to have size of a T*");
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.ptr = reinterpret_cast<const hdf5_type*>(val);
        return w;
    }
};

template <typename T>
struct inspector<const T*> {
    using type = const T*;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

    static constexpr size_t ndim = 1;
    static constexpr size_t recursive_ndim = ndim + inspector<value_type>::recursive_ndim;

    static std::array<size_t, recursive_ndim> getDimensions(const type& /* val */) {
        throw std::string("Not possible to have size of a T*");
    }

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.ptr = reinterpret_cast<const hdf5_type*>(val);
        return w;
    }
};


template <typename T, size_t N>
struct inspector<T[N]> {
    using type = T[N];
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

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

    /* it works because there is only T[][][] currently
       we will fix it one day */
    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.ptr = reinterpret_cast<const hdf5_type*>(&val[0]);
        return w;
    }
};

#ifdef H5_USE_EIGEN
template <typename T, int M, int N>
struct inspector<Eigen::Matrix<T, M, N>> {
    using type = Eigen::Matrix<T, M, N>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;

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

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(static_cast<typename type::Index>(dims[0]),
                   static_cast<typename type::Index>(dims[1]));
    }

    static type alloc(const std::vector<size_t>& dims) {
        type val;
        prepare(val, dims);
        return val;
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.vec = std::vector<hdf5_type>(val.data(), val.data() + val.size());
        return w;
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims.size() < 2) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size() << " dimensions into an eigen-matrix.";
            throw DataSpaceException(os.str());
        }
        type array = alloc(dims);
        memcpy(array.data(), vec_align, compute_total_size(dims) * sizeof(hdf5_type));
        return array;
    }
};
#endif

#ifdef H5_USE_BOOST
template <typename T, size_t Dims>
struct inspector<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

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

    static void prepare(type& val, const std::vector<size_t>& dims) {
        boost::array<typename type::index, Dims> ext;
        std::copy(dims.begin(), dims.begin() + ndim, ext.begin());
        val.resize(ext);
    }

    static type alloc(const std::vector<size_t>& dims) {
        type array;
        prepare(array, dims);
        return array;
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.vec.reserve(compute_total_size(getDimensions(val)));
        size_t size = val.num_elements();
        for (size_t i = 0; i < size; ++i) {
            auto v = inspector<value_type>::serialize(*(val.origin() + i));
            w.vec.insert(w.vec.end(), v.get_pointer(), v.get_pointer() + v.get_size());
        }
        return w;
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims.size() < ndim) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size() << " dimensions into a " << ndim << " boost::multi-array.";
            throw DataSpaceException(os.str());
        }
        type array = alloc(dims);
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        for (size_t i = 0; i < array.num_elements(); ++i) {
            *(array.origin() + i) = inspector<value_type>::unserialize(vec_align + i * subsize,
                                                                       next_dims);
        }
        return array;
    }
};

template <typename T>
struct inspector<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = typename inspector<value_type>::hdf5_type;

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

    static void prepare(type& val, const std::vector<size_t>& dims) {
        val.resize(dims[0], dims[1], false);
    }

    static type alloc(const std::vector<size_t>& dims) {
        type array;
        prepare(array, dims);
        return array;
    }

    static Writer<hdf5_type> serialize(const type& val) {
        Writer<hdf5_type> w;
        w.vec.reserve(compute_total_size(getDimensions(val)));
        size_t size = val.size1() * val.size2();
        for (size_t i = 0; i < size; ++i) {
            auto v = inspector<value_type>::serialize(*(&val(0, 0) + i));
            w.vec.insert(w.vec.end(), v.get_pointer(), v.get_pointer() + v.get_size());
        }
        return w;
    }

    static type unserialize(const hdf5_type* vec_align, const std::vector<size_t>& dims) {
        if (dims.size() < 2) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size() << " dimensions into a " << ndim << " boost::multi-array.";
            throw DataSpaceException(os.str());
        }
        type array = alloc(dims);
        std::vector<size_t> next_dims(dims.begin() + ndim, dims.end());
        size_t subsize = compute_total_size(next_dims);
        size_t size = array.size1() * array.size2();
        for (size_t i = 0; i < size; ++i) {
            *(&array(0, 0) + i) = inspector<value_type>::unserialize(vec_align + i * subsize,
                                                                     next_dims);
        }
        return array;
    }
};
#endif
}
}
