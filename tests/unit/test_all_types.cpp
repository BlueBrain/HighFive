/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <string>

#include <catch2/catch_template_test_macros.hpp>

#include <highfive/highfive.hpp>


#include <type_traits>
#include "tests_high_five.hpp"
#include "data_generator.hpp"
#include "create_traits.hpp"
#include "supported_types.hpp"
#include "compary_arrays.hpp"

using namespace HighFive;

TEMPLATE_TEST_CASE("Scalar in DataSet", "[Types]", bool, std::string) {
    const std::string file_name("rw_dataset_" + typeNameHelper<TestType>() + ".h5");
    const std::string dataset_name("dset");
    TestType t1{};

    {
        // Create a new file using the default property lists.
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);

        // Create the dataset
        DataSet dataset =
            file.createDataSet(dataset_name,
                               DataSpace(1),
                               create_datatype<typename details::inspector<TestType>::base_type>());

        // Write into the initial part of the dataset
        dataset.write(t1);
    }

    // read it back
    {
        File file(file_name, File::ReadOnly);

        TestType value;
        DataSet dataset = file.getDataSet("/" + dataset_name);
        dataset.read(value);
        CHECK(t1 == value);
    }
}

#if HIGHFIVE_CXX_STD >= 17
TEMPLATE_PRODUCT_TEST_CASE("Scalar in std::vector<std::byte>", "[Types]", std::vector, std::byte) {
    const std::string file_name("rw_dataset_vector_" + typeNameHelper<TestType>() + ".h5");
    const std::string dataset_name("dset");
    TestType t1(5, std::byte(0xCD));

    {
        // Create a new file using the default property lists.
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);

        // Create the dataset
        DataSet dataset = file.createDataSet(dataset_name, {5}, create_datatype<std::byte>());

        // Write into the initial part of the dataset
        dataset.write(t1);
    }

    // read it back
    {
        File file(file_name, File::ReadOnly);

        TestType value(5, std::byte(0xCD));
        DataSet dataset = file.getDataSet("/" + dataset_name);
        dataset.read(value);
        CHECK(t1 == value);
        CHECK(value.size() == 5);
    }
}
#endif


template <class Container, class Expected, class Obj>
auto check_read_auto(const Expected& expected, const std::vector<size_t>& dims, const Obj& obj) ->
    typename std::enable_if<!testing::ContainerTraits<Container>::is_view>::type {
    testing::compare_arrays(obj.template read<Container>(), expected, dims);
}

template <class Container, class Expected, class Obj>
auto check_read_auto(const Expected&, const std::vector<size_t>&, const Obj&) ->
    typename std::enable_if<testing::ContainerTraits<Container>::is_view>::type {}

template <class Container, class Expected, class Obj>
void check_read_preallocated(const Expected& expected,
                             const std::vector<size_t>& dims,
                             const Obj& obj) {
    auto actual = testing::DataGenerator<Container>::allocate(dims);
    obj.read(actual);

    testing::compare_arrays(actual, expected, dims);

    testing::ContainerTraits<Container>::deallocate(actual, dims);
}

template <class Container>
void check_read_regular(const std::string& file_name, const std::vector<size_t>& dims) {
    using traits = testing::DataGenerator<Container>;
    using base_type = typename traits::base_type;
    using reference_type = typename testing::MultiDimVector<base_type, traits::rank>::type;

    auto file = File(file_name, File::Truncate);
    auto raw_expected = traits::create(dims);
    auto expected = testing::copy<reference_type>(raw_expected, dims);

    auto dataspace = DataSpace(dims);
    auto attr = testing::AttributeCreateTraits::create<base_type>(file, "dset", dataspace);
    attr.write(expected);

    auto dset = testing::DataSetCreateTraits::create<base_type>(file, "attr", dataspace);
    dset.write(expected);


    SECTION("dset.read<Container>()") {
        check_read_auto<Container>(expected, dims, dset);
    }

    SECTION("dset.read(values)") {
        check_read_preallocated<Container>(expected, dims, dset);
    }

    SECTION("attr.read<Container>()") {
        check_read_auto<Container>(expected, dims, attr);
    }

    SECTION("attr.read(values)") {
        check_read_preallocated<Container>(expected, dims, attr);
    }

    testing::ContainerTraits<reference_type>::deallocate(expected, dims);
    testing::ContainerTraits<Container>::deallocate(raw_expected, dims);
}

template <class Container>
void check_read_regular() {
    const std::string file_name("rw_read_regular" + typeNameHelper<Container>() + ".h5");
    auto dims = testing::DataGenerator<Container>::default_dims();

    check_read_regular<Container>(file_name, dims);
}

TEMPLATE_LIST_TEST_CASE("TestReadRegular", "[read]", testing::supported_array_types) {
    check_read_regular<TestType>();
}

template <class Container, class Write>
void check_writing(const std::vector<size_t>& dims, Write write) {
    using traits = testing::DataGenerator<Container>;
    using base_type = typename traits::base_type;
    using reference_type = typename testing::MultiDimVector<base_type, traits::rank>::type;

    auto values = testing::DataGenerator<Container>::create(dims);
    auto expected = testing::copy<reference_type>(values, dims);

    auto obj = write(values);

    auto actual = testing::DataGenerator<reference_type>::allocate(dims);
    obj.read(actual);

    testing::compare_arrays(actual, expected, dims);

    testing::ContainerTraits<reference_type>::deallocate(actual, dims);
    testing::ContainerTraits<Container>::deallocate(values, dims);
    testing::ContainerTraits<reference_type>::deallocate(expected, dims);
}

template <class CreateTraits, class Container>
auto check_write_auto(File& file, const std::string& name, const std::vector<size_t>& dims) ->
    typename std::enable_if<!testing::ContainerTraits<Container>::is_view>::type {
    auto write_auto = [&](const Container& values) {
        return CreateTraits::create(file, "auto_" + name, values);
    };

    check_writing<Container>(dims, write_auto);
}

template <class CreateTraits, class Container>
auto check_write_auto(File&, const std::string&, const std::vector<size_t>&) ->
    typename std::enable_if<testing::ContainerTraits<Container>::is_view>::type {}

template <class CreateTraits, class Container>
void check_write_deduce_type(File& file, const std::string& name, const std::vector<size_t>& dims) {
    auto write_two_phase_auto = [&](const Container& values) {
        using traits = testing::ContainerTraits<Container>;
        auto dataspace = DataSpace(dims);
        auto h5 = CreateTraits::template create<typename traits::base_type>(file,
                                                                            "two_phase_auto" + name,
                                                                            dataspace);
        h5.write(values);
        return h5;
    };
    check_writing<Container>(dims, write_two_phase_auto);
}

template <class CreateTraits, class Container>
void check_write_manual(File& file, const std::string& name, const std::vector<size_t>& dims) {
    auto write_two_phase = [&](const Container& values) {
        using traits = testing::ContainerTraits<Container>;
        auto datatype = create_datatype<typename traits::base_type>();
        auto dataspace = DataSpace(dims);
        auto h5 = CreateTraits::create(file, "two_phase_" + name, dataspace, datatype);
        h5.write(values);
        return h5;
    };
    check_writing<Container>(dims, write_two_phase);
}

template <class Container>
void check_write_regular(const std::string& file_name, const std::vector<size_t>& dims) {
    auto file = File(file_name, File::Truncate);

    SECTION("createDataSet(name, container)") {
        check_write_auto<testing::DataSetCreateTraits, Container>(file, "dset", dims);
    }

    SECTION("createDataSet(name, container)") {
        check_write_deduce_type<testing::DataSetCreateTraits, Container>(file, "dset", dims);
    }

    SECTION("createDataSet(name, container)") {
        check_write_manual<testing::DataSetCreateTraits, Container>(file, "dset", dims);
    }

    SECTION("createAttribute(name, container)") {
        check_write_auto<testing::AttributeCreateTraits, Container>(file, "attr", dims);
    }

    SECTION("createAttribute(name, container)") {
        check_write_deduce_type<testing::AttributeCreateTraits, Container>(file, "attr", dims);
    }

    SECTION("createAttribute(name, container)") {
        check_write_manual<testing::AttributeCreateTraits, Container>(file, "attr", dims);
    }
}

template <class Container>
void check_write_regular() {
    std::string file_name("rw_write_regular" + typeNameHelper<Container>() + ".h5");
    auto dims = testing::DataGenerator<Container>::default_dims();
    check_write_regular<Container>(file_name, dims);
}

TEMPLATE_LIST_TEST_CASE("TestWriteRegularSTDVector", "[write]", testing::supported_array_types) {
    check_write_regular<TestType>();
}

TEST_CASE("DataGeneratorDefaultDims", "[internal]") {
    SECTION("std::array") {
        auto dims = testing::DataGenerator<std::array<double, 3>>::default_dims();
        REQUIRE(dims.size() == 1);
        CHECK(dims[0] == 3);
    }

    SECTION("std::vector") {
        auto dims = testing::DataGenerator<std::vector<double>>::default_dims();
        REQUIRE(dims.size() == 1);
        CHECK(dims[0] > 0);
    }

    SECTION("std::vector<std::vector>") {
        auto dims = testing::DataGenerator<std::vector<std::vector<double>>>::default_dims();
        REQUIRE(dims.size() == 2);
        CHECK(dims[0] * dims[1] > 0);
    }
}

TEST_CASE("ravel", "[internal]") {
    std::vector<size_t> dims = {2, 4, 5};
    std::vector<size_t> indices = {1, 2, 3};
    size_t flat_index = indices[2] + dims[2] * (indices[1] + dims[1] * indices[0]);

    CHECK(flat_index == testing::ravel(indices, dims));
    CHECK(indices == testing::unravel(flat_index, dims));
}
