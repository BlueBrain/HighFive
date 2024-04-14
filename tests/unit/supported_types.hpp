#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <tuple>

#ifdef HIGHFIVE_TEST_BOOST
#include <boost/multi_array.hpp>
#endif

#ifdef HIGHFIVE_TEST_EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
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

#ifdef HIGHFIVE_TEST_BOOST
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

#ifdef HIGHFIVE_TEST_EIGEN
template <int n, int m, int Option, class C = type_identity>
struct EigenMatrix {
    template <class T>
    using type = Eigen::Matrix<typename C::template type<T>, n, m, Option>;
};

template <int n, int m, int Option, class C = type_identity>
struct EigenArray {
    template <class T>
    using type = Eigen::Array<typename C::template type<T>, n, m, Option>;
};

template <int n, int m, int Option, class C = type_identity>
struct EigenMapArray {
    template <class T>
    using type = Eigen::Map<Eigen::Array<typename C::template type<T>, n, m, Option>>;
};

template <int n, int m, int Option, class C = type_identity>
struct EigenMapMatrix {
    template <class T>
    using type = Eigen::Map<Eigen::Matrix<typename C::template type<T>, n, m, Option>>;
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
using all_numeric_scalar_types = std::tuple<
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


// To reduce the explosion of combinations, we don't always need
// to test against every numeric scalar type. These three should
// suffice.
using some_numeric_scalar_types = std::tuple<char, int, double>;

using all_scalar_types = typename ConcatenateTuples<all_numeric_scalar_types, std::tuple<bool, std::string>>::type;
using some_scalar_types = typename ConcatenateTuples<some_numeric_scalar_types, std::tuple<bool, std::string>>::type;

using scalar_types_boost = some_numeric_scalar_types;
using scalar_types_eigen = some_numeric_scalar_types;

using supported_array_types = typename ConcatenateTuples<
#ifdef HIGHFIVE_TEST_BOOST
  typename ContainerProduct<BoostMultiArray<3>, scalar_types_boost>::type,
  typename ContainerProduct<STDVector<BoostMultiArray<3>>, scalar_types_boost>::type,
  typename ContainerProduct<STDArray<5, BoostMultiArray<3>>, scalar_types_boost>::type,

  typename ContainerProduct<BoostUblasMatrix<>, scalar_types_boost>::type,
  typename ContainerProduct<STDVector<BoostUblasMatrix<>>, scalar_types_boost>::type,
  typename ContainerProduct<STDArray<5, BoostUblasMatrix<>>, scalar_types_boost>::type,
#endif
#ifdef HIGHFIVE_TEST_EIGEN
  typename ContainerProduct<EigenMatrix<3, 5, Eigen::ColMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenMatrix<3, 5, Eigen::RowMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenMatrix<Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenMatrix<Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenArray<3, 5, Eigen::ColMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenArray<3, 5, Eigen::RowMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenArray<Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>, scalar_types_eigen>::type,
  typename ContainerProduct<EigenArray<Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>, scalar_types_eigen>::type,
  std::tuple<Eigen::Vector2d, Eigen::VectorXd>,
  typename ContainerProduct<EigenMapMatrix<3, 5, Eigen::ColMajor>, scalar_types_eigen>::type,

  typename ContainerProduct<STDVector<EigenMatrix<3, 5, Eigen::ColMajor>>, scalar_types_eigen>::type,
  typename ContainerProduct<STDVector<EigenArray<Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>>, scalar_types_eigen>::type,
  std::tuple<std::vector<Eigen::Vector3d>, std::vector<Eigen::VectorXd>>,

  typename ContainerProduct<STDArray<7, EigenMatrix<3, 5, Eigen::RowMajor>>, scalar_types_eigen>::type,
  typename ContainerProduct<STDArray<7, EigenArray<Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>, scalar_types_eigen>::type,
  std::tuple<std::array<Eigen::VectorXd, 7>>,
#endif
  typename ContainerProduct<STDVector<>, all_scalar_types>::type,
  typename ContainerProduct<STDVector<STDVector<>>, some_scalar_types>::type,
  typename ContainerProduct<STDVector<STDVector<STDVector<>>>, some_scalar_types>::type,
  typename ContainerProduct<STDVector<STDVector<STDVector<STDVector<>>>>, some_scalar_types>::type,
  typename ContainerProduct<STDArray<3>, some_scalar_types>::type,
  typename ContainerProduct<STDArray<7, STDArray<5>>, some_scalar_types>::type,
  typename ContainerProduct<STDVector<STDArray<5>>, some_scalar_types>::type,
  typename ContainerProduct<STDArray<7, STDVector<>>, some_scalar_types>::type
>::type;

// clang-format on

}  // namespace testing
}  // namespace HighFive
