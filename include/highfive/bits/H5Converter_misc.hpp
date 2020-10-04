/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

/*  INTERFACES
struct data_converter {
  using value_type;
  using hdf5_type;
  using h5_type;

  data_converter(const DataSpace& space, const std::vector<size_t>& dims);

  // Size for HDF5
  static std::vector<size_t> get_size(const value_type&);

  // Before reading for creating a type
  void allocate(value_type&) const;

  // Only for continuous
  // Reading
  static hdf5_type* get_pointer(value_type&);
  // Writing
  static const hdf5_type* get_pointer(const value_type&);

  // Before writing non-continuous
  inline void serialize(const value_type&, hdf5_type*);

  // After reading non-continuous
  inline void unserialize(value_type&, const hdf5_type*);

  static constexpr size_t number_of_dims;

  // Number of elements for C++
  const size_t _number_of_elements;
};
*/

#ifndef H5CONVERTER_MISC_HPP
#define H5CONVERTER_MISC_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>

#ifdef H5_USE_BOOST
// starting Boost 1.64, serialization header must come before ublas
#include <boost/serialization/vector.hpp>
#include <boost/multi_array.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#endif

#include <H5Dpublic.h>
#include <H5Ppublic.h>

#include "../H5Reference.hpp"
#include "../H5DataType.hpp"
#include "H5Utils.hpp"

namespace HighFive {

namespace details {
template <typename T>
struct h5_pod :
    std::is_pod<T> {};

template <>
struct h5_pod<bool> :
    std::false_type {};

// contiguous pair of T
// template <typename T>
// struct h5_pod<std::complex<T>> :
//     std::true_type {};

template <typename T>
struct h5_continuous :
    std::false_type {};

template <typename S>
struct h5_continuous<std::vector<S>> :
    std::integral_constant<bool, h5_pod<S>::value> {};

template <typename T, size_t N>
struct h5_continuous<T[N]> :
    std::integral_constant<bool, h5_pod<T>::value> {};

template <typename S, size_t N>
struct h5_continuous<std::array<S, N>> :
    std::integral_constant<bool, h5_pod<S>::value> {};

template <typename T, size_t Dims>
struct h5_continuous<boost::multi_array<T, Dims>> :
    std::integral_constant<bool, h5_pod<T>::value> {};

template <typename T>
struct h5_continuous<boost::numeric::ublas::matrix<T>> :
    std::integral_constant<bool, h5_pod<T>::value> {};

template <typename T>
struct h5_non_continuous :
    std::integral_constant< bool, !h5_continuous<T>::value> {};

size_t get_number_of_elements(const std::vector<size_t>& dims) {
    return std::accumulate(dims.begin(), dims.end(), size_t{1u}, std::multiplies<size_t>());
}

template <typename T>
struct manipulator {
    using type = T;
    using hdf5_type = T;
    using h5_type = type;

    static type from_hdf5(const hdf5_type* data) {
        return type(*data);
    }

    static void to_hdf5(const type& scalar, hdf5_type* data) {
        *data = scalar;
    }

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{};
    }

    static size_t flat_size(const type& val) {
        return 1;
    }

    static hdf5_type* first(type& val) {
        return &val;
    }

    static const hdf5_type* first(const type& val) {
        return &val;
    }

    static hdf5_type* data(type& val) {
        return &val;
    }

    static const hdf5_type* data(const type& val) {
        return &val;
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        // Nothing to do
    }

    static const size_t n_dims = 0;
};

template <typename T>
struct manipulator<std::complex<T>> {
    using type = std::complex<T>;
    using hdf5_type = std::complex<T>;
    using h5_type = type;

    static type from_hdf5(const hdf5_type* data) {
        return type(*data);
    }

    static void to_hdf5(const type& scalar, hdf5_type* data) {
        *data = scalar;
    }

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{};
    }

    static size_t flat_size(const type& val) {
        return 1;
    }

    static hdf5_type* first(type& val) {
        return nullptr;
    }

    static const hdf5_type* first(const type& val) {
        return nullptr;
    }

    static hdf5_type* data(type& val) {
        // .data() return a const pointer until c++17
        throw std::string("Invalid get_pointer on std::string");
    }

    static const hdf5_type* data(const type& val) {
        static hdf5_type _c_str = val.c_str();
        return &_c_str;
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        // Nothing to do
    }

    static const size_t n_dims = 0;
    static const size_t rn_dims = n_dims;
};

template <size_t N>
struct manipulator<FixedLengthString<N>> {
    using type = FixedLengthString<N>;
    using hdf5_type = char;
    using h5_type = std::array<char, N>;

    static type from_hdf5(const hdf5_type* data) {
        return type(*data);
    }

    static void to_hdf5(const type& scalar, hdf5_type* data) {
        *data = scalar.data();
    }

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{};
    }

    static size_t flat_size(const type& val) {
        return N;
    }

    static hdf5_type* first(type& val) {
        return nullptr;
    }

    static const hdf5_type* first(const type& val) {
        return nullptr;
    }

    static hdf5_type* data(type& val) {
        return val.data();
    }

    static const hdf5_type* data(const type& val) {
        return val.data();
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        // cannot resize a fixed length
    }

    static const size_t n_dims = 0;
    static const size_t rn_dims = n_dims;
};
template <>
struct manipulator<std::string> {
    using type = std::string;
    using hdf5_type = const char*;
    using h5_type = type;

    static type from_hdf5(const hdf5_type* data) {
        return type(*data);
    }

    static void to_hdf5(const type& scalar, hdf5_type* data) {
        hdf5_type _c_str = scalar.c_str();
        *data = _c_str;
    }

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{};
    }

    static size_t flat_size(const type& val) {
        return val.size();
    }

    static hdf5_type* first(type& val) {
        return nullptr;
    }

    static const hdf5_type* first(const type& val) {
        return nullptr;
    }

    static hdf5_type* data(type& val) {
        // .data() return a const pointer until c++17
        throw std::string("Invalid get_pointer on std::string");
    }

    static const hdf5_type* data(const type& val) {
        static hdf5_type _c_str = val.c_str();
        return &_c_str;
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        // Nothing to do
    }

    static const size_t n_dims = 0;
    static const size_t rn_dims = n_dims;
};

template <>
struct manipulator<Reference> {
    using type = Reference;
    using hdf5_type = hobj_ref_t;
    using h5_type = type;

    static type from_hdf5(const hdf5_type* data) {
        return type(*data);
    }

    static void to_hdf5(const type& scalar, hdf5_type* data) {
        scalar.create_ref(data);
    }

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{};
    }

    static size_t flat_size(const type& val) {
        return 1;
    }

    static hdf5_type* first(type& val) {
        return nullptr;
    }

    static const hdf5_type* first(const type& val) {
        return nullptr;
    }

    static hdf5_type* data(type& val) {
        return nullptr;
    }

    static const hdf5_type* data(const type& val) {
        return nullptr;
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        // Nothing to do
    }

    static const size_t n_dims = 0;
    static const size_t rn_dims = n_dims;
};

template <typename T>
struct manipulator<std::vector<T>> {
    using type = std::vector<T>;
    using value_type = typename type::value_type;
    using hdf5_type = typename manipulator<value_type>::hdf5_type;
    using h5_type = typename manipulator<value_type>::h5_type;

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{val.size()};
    }

    static size_t flat_size(const type& val) {
        return val.size();
    }

    static hdf5_type* first(type& val) {
        return manipulator<value_type>::first(val[0]);
    }

    static const hdf5_type* first(const type& val) {
        return manipulator<value_type>::first(val[0]);
    }

    static value_type* data(type& val) {
        return val.data();
    }

    static const value_type* data(const type& val) {
        return val.data();
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        val.resize(count[0]);
    }

    static const size_t n_dims = 1;
    static const size_t rn_dims = n_dims + manipulator<value_type>::n_dims;
};

template <typename T, size_t N>
struct manipulator<std::array<T, N>> {
    using type = std::array<T, N>;
    using value_type = typename type::value_type;
    using hdf5_type = typename manipulator<value_type>::hdf5_type;
    using h5_type = typename manipulator<value_type>::h5_type;

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>{val.size()};
    }

    static size_t flat_size(const type& val) {
        return val.size();
    }

    static hdf5_type* first(type& val) {
        return manipulator<value_type>::first(val[0]);
    }

    static const hdf5_type* first(const type& val) {
        return manipulator<value_type>::first(val[0]);
    }

    static value_type* data(type& val) {
        return val.data();
    }

    static const value_type* data(const type& val) {
        return val.data();
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);
        assert(count[0] == N);
    }

    static const size_t n_dims = 1;
    static const size_t rn_dims = n_dims + manipulator<value_type>::n_dims;
};

template <typename T, size_t Dims>
struct manipulator<boost::multi_array<T, Dims>> {
    using type = boost::multi_array<T, Dims>;
    using value_type = typename type::element;
    using hdf5_type = typename manipulator<value_type>::hdf5_type;
    using h5_type = typename manipulator<value_type>::h5_type;

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>(val.shape(), val.shape() + Dims);
    }

    static size_t flat_size(const type& val) {
        return val.num_elements();
    }

    static hdf5_type* first(type& val) {
        return manipulator<value_type>::first(val.data()[0]);
    }

    static const hdf5_type* first(const type& val) {
        return manipulator<value_type>::first(val.data()[0]);
    }

    static value_type* data(type& val) {
        return val.data();
    }

    static const value_type* data(const type& val) {
        return val.data();
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);

        boost::array<typename type::index, Dims> ext;
        std::copy(count.begin(), count.end(), ext.begin());
        val.resize(ext);
    }

    static const size_t n_dims = Dims;
    static const size_t rn_dims = n_dims + manipulator<value_type>::n_dims;
};

template <typename T>
struct manipulator<boost::numeric::ublas::matrix<T>> {
    using type = boost::numeric::ublas::matrix<T>;
    using value_type = typename type::value_type;
    using hdf5_type = typename manipulator<value_type>::hdf5_type;
    using h5_type = typename manipulator<value_type>::h5_type;

    static std::vector<size_t> size(const type& val) {
        return std::vector<size_t>(val.size1(), val.size2());
    }

    static size_t flat_size(const type& val) {
        return val.size1() * val.size2();
    }

    static hdf5_type* first(type& val) {
        return manipulator<value_type>::first(val(0, 0));
    }

    static const hdf5_type* first(const type& val) {
        return manipulator<value_type>::first(val(0, 0));
    }

    static value_type* data(type& val) {
        return &val(0, 0);
    }

    static const value_type* data(const type& val) {
        return &val(0, 0);
    }

    static void resize(type& val, const std::vector<size_t>& count) {
        assert(count.size() == n_dims);

        val.resize(count[0], count[1], false);
        val(0, 0) = 0;
    }

    static const size_t n_dims = 2;
    static const size_t rn_dims = n_dims + manipulator<value_type>::n_dims;
};

template <typename T, size_t N>
struct read_data_converter {
    using manip = manipulator<T>;
    using inner_converter = read_data_converter<typename manip::value_type, manipulator<typename manip::value_type>::n_dims>;

    using type = typename manip::type;
    using hdf5_type = typename manip::hdf5_type;

    void resize(type& val, const std::vector<size_t>& dims) {
        manip::resize(val, std::vector<size_t>(dims.begin(), dims.begin() + manip::n_dims));
        for (unsigned int i = 0; i < manip::flat_size(val); ++i) {
            _inner_converter.resize(manip::data(val)[i], std::vector<size_t>(dims.begin() + manip::n_dims, dims.end()));
        }
    }

    inline void unserialize(type& vec, const hdf5_type* data) {
        const size_t inner_size = manipulator<typename manip::value_type>::flat_size(manip::data(vec)[0]);
        for (unsigned int i = 0; i < manip::flat_size(vec); ++i) {
            _inner_converter.unserialize(manip::data(vec)[i], data + inner_size * i); 
        }
    }

    hdf5_type* data(type& val) {
        return manip::first(val);
    }

    inner_converter _inner_converter;
};

template <typename T>
struct read_data_converter<T, 0> {
    using manip = manipulator<T>;

    using type = typename manip::type;
    using hdf5_type = typename manip::hdf5_type;

    void resize(type& val, const std::vector<size_t>& dims) {
        manip::resize(val, std::vector<size_t>(dims.begin(), dims.begin() + manip::n_dims));
    }

    inline void unserialize(type& scalar, const hdf5_type* data) {
        scalar = manip::from_hdf5(data);
    }

    hdf5_type* data(type& val) {
        return manip::first(val);
    }
};

template <typename T>
struct static_data_converter {
    using manip = manipulator<T>;
    using hdf5_type = typename manip::hdf5_type;
    using h5_type = typename manip::h5_type;

    static std::vector<size_t> dims(const T& val) {
        return manip::size(val);
    }

    static constexpr size_t n_dims = manip::n_dims;
    static constexpr size_t rn_dims = manip::n_dims;
};

template <typename T, size_t N>
struct write_data_converter {
    using manip = manipulator<T>;
    using inner_converter = write_data_converter<typename manip::value_type, manipulator<typename manip::value_type>::n_dims>;

    using type = typename manip::type;
    using hdf5_type = typename manip::hdf5_type;

    inline void serialize(const type& scalar, hdf5_type* data) {
        const size_t inner_size = manipulator<typename manip::value_type>::flat_size(manip::data(scalar)[0]);
        for (size_t i = 0; i < manip::flat_size(scalar); ++i) {
            _inner_converter.serialize(manip::data(scalar)[i], data + inner_size * i);
        }
    }

    const hdf5_type* data(const type& val) {
        return manip::first(val);
    }

    inner_converter _inner_converter;
};

template <typename T>
struct write_data_converter<T, 0> {
    using manip = manipulator<T>;

    using type = typename manip::type;
    using hdf5_type = typename manip::hdf5_type;

    inline void serialize(const type& scalar, hdf5_type* data) {
        manip::to_hdf5(scalar, data);
    }

    const hdf5_type* data(const type& val) {
        return manip::data(val);
    }
};
}  // namespace details

std::vector<size_t> real_dims(const std::vector<size_t>& from, size_t to_size) {
    if (from.size() <= to_size) { // We can do nothing
        return from;
    }

    auto distance = static_cast<long int>(from.size() - to_size);
    bool correct = true;
    for (long int i = 0; i < distance; ++i) {
        if (from[static_cast<size_t>(i)] != 1) {
            correct = false;
            break;
        }
    }
    if (correct) {
        return std::vector<size_t>(from.begin() + distance, from.end());
    }

    correct = true;
    for (long int i = distance; i > 0; --i) {
        if (from[static_cast<size_t>(i)] != 1) {
            correct = false;
            break;
        }
    }
    if (correct) {
        return std::vector<size_t>(from.begin(), from.end() - distance);
    }
    return from;
}

template <typename T, typename Enable = void>
class TransformRead;

template <typename T>
class TransformRead<T, typename std::enable_if<details::h5_non_continuous<T>::value>::type> {
  public:
    using hdf5_type = typename details::static_data_converter<T>::hdf5_type;
    using Conv = details::read_data_converter<T, n_dims>;
    using h5_type = typename details::static_data_converter<T>::h5_type;

    TransformRead(const DataSpace& space)
        : _space(space)
        , _dims(real_dims(space.getDimensions(), n_dims))
    {
#ifdef H5_ENABLE_ERROR_ON_COPY
#error You are using a non contiguous data type and so data will be copied
#endif
    }

    hdf5_type* get_pointer() {
        _vec.resize(details::get_number_of_elements(_dims));
        return _vec.data();
    }

    T transform_read() {
        T _data;
        _converter.resize(_data, _dims);
        _converter.unserialize(_data, _vec.data());
        return _data;
    }

    DataType _h5_type = create_and_check_datatype<h5_type>();
    static constexpr size_t n_dims = details::static_data_converter<T>::n_dims;

  private:
    // Continuous vector for data
    std::vector<hdf5_type> _vec;

    const DataSpace& _space;
    std::vector<size_t> _dims;
    Conv _converter;
};

template<typename T>
class TransformRead<T, typename std::enable_if<details::h5_continuous<T>::value>::type> {
  public:
    using hdf5_type = typename details::static_data_converter<T>::hdf5_type;
    using Conv = details::read_data_converter<T, details::manipulator<T>::n_dims>;
    using h5_type = typename details::static_data_converter<T>::h5_type;

    TransformRead(const DataSpace& space)
        : _space(space)
        , _dims(real_dims(space.getDimensions(), n_dims))
    {}

    hdf5_type* get_pointer() {
        _converter.resize(_data, _dims);
        return _converter.data(_data);
    }

    T transform_read() {
        return _data;
    }

    DataType _h5_type = create_and_check_datatype<h5_type>();
    static constexpr size_t n_dims = details::static_data_converter<T>::n_dims;

  private:
    T _data;
    const DataSpace& _space;
    std::vector<size_t> _dims;
    Conv _converter;
};

template <typename T, typename Enable = void>
class TransformWrite;

template <typename T>
class TransformWrite<T, typename std::enable_if<details::h5_non_continuous<T>::value>::type> {
  public:
    using hdf5_type = typename details::static_data_converter<T>::hdf5_type;
    using Conv = details::write_data_converter<T, details::manipulator<T>::n_dims>;
    using h5_type = typename details::static_data_converter<T>::h5_type;

    TransformWrite(const DataSpace& space, const T& value)
        : _dims(real_dims(space.getDimensions(), n_dims))
    {
#ifdef H5_ENABLE_ERROR_ON_COPY
#error You are using a non contiguous data type and so data will be copied
#endif
        _vec.resize(details::get_number_of_elements(_dims));
        _converter.serialize(value, _vec.data());
    }

    const hdf5_type* get_pointer() const {
        return _vec.data();
    }

  private:
    std::vector<size_t> _dims;
    Conv _converter;
    std::vector<hdf5_type> _vec;
  public:
    static constexpr size_t n_dims = details::static_data_converter<T>::n_dims;
    DataType _h5_type = create_and_check_datatype<h5_type>();
};

template <typename T>
class TransformWrite<T, typename std::enable_if<details::h5_continuous<T>::value>::type> {
  public:
    using hdf5_type = typename details::static_data_converter<T>::hdf5_type;
    using Conv = details::write_data_converter<T, details::manipulator<T>::n_dims>;
    using h5_type = typename details::static_data_converter<T>::h5_type;

    TransformWrite(const DataSpace& space, const T& value)
        : _dims(real_dims(space.getDimensions(), n_dims))
        , _data(value)
    {}

    const hdf5_type* get_pointer() {
        return _converter.data(_data);
    }

  private:
    std::vector<size_t> _dims;
    Conv _converter;
    const T& _data;
  public:
    static constexpr size_t n_dims = details::static_data_converter<T>::n_dims;
    DataType _h5_type = create_and_check_datatype<h5_type>();
};

// Wrappers to have template deduction, that are not available with class before C++17
template <typename T>
TransformWrite<T> make_transform_write(const DataSpace& space, const T& value) {
    return TransformWrite<T>{space, value};
}

template <typename T>
TransformRead<T> make_transform_read(const DataSpace& space) {
    return TransformRead<T>{space};
}
}  // namespace HighFive

#ifdef H5_USE_EIGEN
#include "H5ConverterEigen_misc.hpp"
#endif

#endif // H5CONVERTER_MISC_HPP
