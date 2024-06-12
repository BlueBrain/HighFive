/*
 *  Copyright (c), 2017-2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <highfive/highfive.hpp>
#include "tests_high_five.hpp"
#include "create_traits.hpp"

#ifdef HIGHFIVE_TEST_BOOST
#include <highfive/boost.hpp>
#endif

#ifdef HIGHFIVE_TEST_EIGEN
#include <highfive/eigen.hpp>
#endif

#ifdef HIGHFIVE_TEST_SPAN
#include <highfive/span.hpp>
#endif

using namespace HighFive;
using Catch::Matchers::Equals;


template <int n_dim>
struct CreateEmptyVector;

template <>
struct CreateEmptyVector<1> {
    using container_type = std::vector<int>;

    static container_type create(const std::vector<size_t>& dims) {
        return container_type(dims[0], 2);
    }
};

template <int n_dim>
struct CreateEmptyVector {
    using container_type = std::vector<typename CreateEmptyVector<n_dim - 1>::container_type>;

    static container_type create(const std::vector<size_t>& dims) {
        auto subdims = std::vector<size_t>(dims.begin() + 1, dims.end());
        return container_type(dims[0], CreateEmptyVector<n_dim - 1>::create(subdims));
    }
};

#ifdef HIGHFIVE_TEST_BOOST
template <int n_dim>
struct CreateEmptyBoostMultiArray {
    using container_type = boost::multi_array<int, static_cast<long unsigned>(n_dim)>;

    static container_type create(const std::vector<size_t>& dims) {
        auto container = container_type(dims);

        auto raw_data = std::vector<int>(compute_total_size(dims));
        container.assign(raw_data.begin(), raw_data.end());

        return container;
    }
};
#endif


#ifdef HIGHFIVE_TEST_EIGEN
struct CreateEmptyEigenVector {
    using container_type = Eigen::VectorXi;

    static container_type create(const std::vector<size_t>& dims) {
        return container_type::Constant(int(dims[0]), 2);
    }
};

struct CreateEmptyEigenMatrix {
    using container_type = Eigen::MatrixXi;

    static container_type create(const std::vector<size_t>& dims) {
        return container_type::Constant(int(dims[0]), int(dims[1]), 2);
    }
};
#endif

template <class Container>
void check_empty_dimensions(const Container& container, const std::vector<size_t>& expected_dims) {
    auto deduced_dims = details::inspector<Container>::getDimensions(container);

    REQUIRE(expected_dims.size() == deduced_dims.size());

    // The dims after hitting the first `0` are finicky. We allow those to be deduced as either `1`
    // or what the original dims said. The `1` allows broadcasting, the "same as original" enables
    // statically sized objects, which conceptually have dims, even if there's no object.
    bool allow_one = false;
    for (size_t i = 0; i < expected_dims.size(); ++i) {
        REQUIRE(((expected_dims[i] == deduced_dims[i]) || (allow_one && (deduced_dims[i] == 1ul))));

        if (expected_dims[i] == 0) {
            allow_one = true;
        }
    }
}

template <class CreateContainer>
void check_empty_dimensions(const std::vector<size_t>& dims) {
    auto input_data = CreateContainer::create(dims);
    check_empty_dimensions(input_data, dims);
}

template <class ReadWriteInterface, class CreateContainer>
void check_empty_read_write_cycle(const std::vector<size_t>& dims) {
    using container_type = typename CreateContainer::container_type;

    const std::string file_name("h5_empty_attr.h5");
    const std::string dataset_name("dset");
    File file(file_name, File::Truncate);

    auto input_data = CreateContainer::create(dims);
    ReadWriteInterface::create(file, dataset_name, input_data);

    SECTION("read; one-dimensional vector (empty)") {
        auto output_data = CreateEmptyVector<1>::create({0ul});

        ReadWriteInterface::get(file, dataset_name).reshapeMemSpace({0ul}).read(output_data);
        check_empty_dimensions(output_data, {0ul});
    }

    SECTION("read; pre-allocated (empty)") {
        auto output_data = CreateContainer::create(dims);
        ReadWriteInterface::get(file, dataset_name).reshapeMemSpace(dims).read(output_data);

        check_empty_dimensions(output_data, dims);
    }

    SECTION("read; pre-allocated (oversized)") {
        auto oversize_dims = std::vector<size_t>(dims.size(), 2ul);
        auto output_data = CreateContainer::create(oversize_dims);
        ReadWriteInterface::get(file, dataset_name).reshapeMemSpace(dims).read(output_data);

        check_empty_dimensions(output_data, dims);
    }

    SECTION("read; auto-allocated") {
        auto output_data = ReadWriteInterface::get(file, dataset_name)
                               .reshapeMemSpace(dims)
                               .template read<container_type>();
        check_empty_dimensions(output_data, dims);
    }
}

template <class CreateContainer>
void check_empty_dataset(const std::vector<size_t>& dims) {
    check_empty_read_write_cycle<testing::DataSetCreateTraits, CreateContainer>(dims);
}

template <class CreateContainer>
void check_empty_attribute(const std::vector<size_t>& dims) {
    check_empty_read_write_cycle<testing::AttributeCreateTraits, CreateContainer>(dims);
}

template <class CreateContainer>
void check_empty_everything(const std::vector<size_t>& dims) {
    SECTION("Empty dimensions") {
        check_empty_dimensions<CreateContainer>(dims);
    }

    SECTION("Empty datasets") {
        check_empty_dataset<CreateContainer>(dims);
    }

    SECTION("Empty attribute") {
        check_empty_attribute<CreateContainer>(dims);
    }
}

#ifdef HIGHFIVE_TEST_EIGEN
template <int ndim>
void check_empty_eigen(const std::vector<size_t>&) {}

template <>
void check_empty_eigen<1>(const std::vector<size_t>& dims) {
    SECTION("Eigen::Vector") {
        check_empty_everything<CreateEmptyEigenVector>({dims[0], 1ul});
    }
}

template <>
void check_empty_eigen<2>(const std::vector<size_t>& dims) {
    SECTION("Eigen::Matrix") {
        check_empty_everything<CreateEmptyEigenMatrix>(dims);
    }
}
#endif

template <int ndim>
void check_empty(const std::vector<size_t>& dims) {
    REQUIRE(dims.size() == ndim);

    SECTION("std::vector") {
        check_empty_everything<CreateEmptyVector<ndim>>(dims);
    }

#ifdef HIGHFIVE_TEST_BOOST
    SECTION("boost::multi_array") {
        check_empty_everything<CreateEmptyBoostMultiArray<ndim>>(dims);
    }
#endif

#ifdef HIGHFIVE_TEST_EIGEN
    check_empty_eigen<ndim>(dims);
#endif
}

TEST_CASE("Empty arrays") {
    SECTION("one-dimensional") {
        check_empty<1>({0ul});
    }

    SECTION("two-dimensional") {
        std::vector<std::vector<size_t>> testcases{{0ul, 1ul}, {1ul, 0ul}};

        for (const auto& dims: testcases) {
            SECTION(details::format_vector(dims)) {
                check_empty<2>(dims);
            }
        }
    }

    SECTION("three-dimensional") {
        std::vector<std::vector<size_t>> testcases{{0ul, 1ul, 1ul},
                                                   {1ul, 1ul, 0ul},
                                                   {1ul, 0ul, 1ul}};

        for (const auto& dims: testcases) {
            SECTION(details::format_vector(dims)) {
                check_empty<3>(dims);
            }
        }
    }
}
