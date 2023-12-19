
#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <tuple>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#endif

namespace HighFive {
namespace testing {

struct type_identity {
    template <class T>
    using type = T;
};

template <class C = type_identity>
struct STDVector {
    template <class T>
    using type = std::vector<typename C::template type<T>>;
};

template <size_t n, class C = type_identity>
struct STDArray {
    template <class T>
    using type = std::array<typename C::template type<T>, n>;
};

#ifdef H5_USE_BOOST
template <size_t n, class C = type_identity>
struct BoostMultiArray {
    template <class T>
    using type = boost::multi_array<typename C::template type<T>, 4>;
};

template <class C = type_identity>
struct BoostUblasMatrix {
    template <class T>
    using type = boost::numeric::ublas::matrix<typename C::template type<T>>;
};
#endif

template <class C, class Tuple>
struct ContainerProduct;

template <class C, class... ScalarTypes>
struct ContainerProduct<C, std::tuple<ScalarTypes...>> {
    using type = std::tuple<typename C::template type<ScalarTypes>...>;
};

template <class... Tuples>
struct ConcatenateTuples;

template <class... Args1, class... Args2, class... Tuples>
struct ConcatenateTuples<std::tuple<Args1...>, std::tuple<Args2...>, Tuples...> {
    using type = typename ConcatenateTuples<std::tuple<Args1..., Args2...>, Tuples...>::type;
};

template <class... Args1>
struct ConcatenateTuples<std::tuple<Args1...>> {
    using type = std::tuple<Args1...>;
};

// clang-format off
using numeric_scalar_types = std::tuple<
    int,
    unsigned int,
    long,
    unsigned long,
    unsigned char,
    char,
    float,
    double,
    long long,
    unsigned long long
>;

using scalar_types = typename ConcatenateTuples<numeric_scalar_types, std::tuple<bool, std::string>>::type;
using scalar_types_boost = typename ConcatenateTuples<numeric_scalar_types, std::tuple<bool>>::type;

using supported_array_types = typename ConcatenateTuples<
#ifdef H5_USE_BOOST
  typename ContainerProduct<BoostMultiArray<3>, scalar_types_boost>::type,
  typename ContainerProduct<STDVector<BoostMultiArray<3>>, scalar_types_boost>::type,
  typename ContainerProduct<STDArray<5, BoostMultiArray<3>>, scalar_types_boost>::type,

  typename ContainerProduct<BoostUblasMatrix<>, scalar_types_boost>::type,
  typename ContainerProduct<STDVector<BoostUblasMatrix<>>, scalar_types_boost>::type,
  typename ContainerProduct<STDArray<5, BoostUblasMatrix<>>, scalar_types_boost>::type,
#endif
  typename ContainerProduct<STDVector<>, scalar_types>::type,
  typename ContainerProduct<STDVector<STDVector<>>, scalar_types>::type,
  typename ContainerProduct<STDVector<STDVector<STDVector<>>>, scalar_types>::type,
  typename ContainerProduct<STDVector<STDVector<STDVector<STDVector<>>>>, scalar_types>::type,
  typename ContainerProduct<STDArray<3>, scalar_types>::type,
  typename ContainerProduct<STDArray<7, STDArray<5>>, scalar_types>::type,
  typename ContainerProduct<STDVector<STDArray<5>>, scalar_types>::type,
  typename ContainerProduct<STDArray<7, STDVector<>>, scalar_types>::type
>::type;

// clang-format on

}  // namespace testing
}  // namespace HighFive
