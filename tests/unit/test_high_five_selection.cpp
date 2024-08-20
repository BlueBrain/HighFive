/*
 *  Copyright (c), 2017-2023, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <highfive/highfive.hpp>
#include "tests_high_five.hpp"
#include "data_generator.hpp"

using namespace HighFive;
using Catch::Matchers::Equals;

template <typename T>
void selectionArraySimpleTest() {
    typedef typename std::vector<T> Vector;

    std::ostringstream filename;
    filename << "h5_rw_select_test_" << typeNameHelper<T>() << "_test.h5";

    const size_t size_x = 10;
    const size_t offset_x = 2, count_x = 5;

    const std::string dataset_name("dset");

    Vector values(size_x);

    ContentGenerate<T> generator;
    std::generate(values.begin(), values.end(), generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(dataset_name, DataSpace::From(values));

    dataset.write(values);

    file.flush();

    // select slice
    {
        // read it back
        Vector result;
        std::vector<size_t> offset{offset_x};
        std::vector<size_t> size{count_x};

        Selection slice = dataset.select(offset, size);

        CHECK(slice.getSpace().getDimensions()[0] == size_x);
        CHECK(slice.getMemSpace().getDimensions()[0] == count_x);

        slice.read(result);

        CHECK(result.size() == 5);

        for (size_t i = 0; i < count_x; ++i) {
            REQUIRE(values[i + offset_x] == result[i]);
        }
    }

    // select cherry pick
    {
        // read it back
        Vector result;
        std::vector<size_t> ids{1, 3, 4, 7};

        Selection slice = dataset.select(ElementSet(ids));

        CHECK(slice.getSpace().getDimensions()[0] == size_x);
        CHECK(slice.getMemSpace().getDimensions()[0] == ids.size());

        slice.read(result);

        CHECK(result.size() == ids.size());

        for (size_t i = 0; i < ids.size(); ++i) {
            const std::size_t id = ids[i];
            REQUIRE(values[id] == result[i]);
        }
    }
}

TEST_CASE("selectionArraySimpleString") {
    selectionArraySimpleTest<std::string>();
}

TEMPLATE_LIST_TEST_CASE("selectionArraySimple", "[template]", dataset_test_types) {
    selectionArraySimpleTest<TestType>();
}

TEST_CASE("selectionByElementMultiDim") {
    const std::string file_name("h5_test_selection_multi_dim.h5");
    // Create a 2-dim dataset
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    std::vector<size_t> dims{3, 3};

    auto set = file.createDataSet("test", DataSpace(dims), AtomicType<int>());
    int values[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    set.write(values);

    {
        int value;
        set.select(ElementSet{{1, 1}}).read(value);
        CHECK(value == 5);
    }

    {
        int value[2];
        set.select(ElementSet{0, 0, 2, 2}).read(value);
        CHECK(value[0] == 1);
        CHECK(value[1] == 9);
    }

    {
        int value[2];
        set.select(ElementSet{{0, 1}, {1, 2}}).read(value);
        CHECK(value[0] == 2);
        CHECK(value[1] == 6);
    }

    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(set.select(ElementSet{0, 1, 2}), DataSpaceException);
    }
}

template <typename T>
void columnSelectionTest() {
    std::ostringstream filename;
    filename << "h5_rw_select_column_test_" << typeNameHelper<T>() << "_test.h5";

    const size_t x_size = 10;
    const size_t y_size = 7;

    const std::string dataset_name("dset");

    T values[x_size][y_size];

    ContentGenerate<T> generator;
    generate2D(values, x_size, y_size, generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims{x_size, y_size};

    DataSpace dataspace(dims);
    // Create a dataset with arbitrary type
    DataSet dataset = file.createDataSet<T>(dataset_name, dataspace);

    dataset.write(values);

    file.flush();

    std::vector<size_t> columns{1, 3, 5};

    Selection slice = dataset.select(columns);
    T result[x_size][3];
    slice.read(result);

    CHECK(slice.getSpace().getDimensions()[0] == x_size);
    CHECK(slice.getMemSpace().getDimensions()[0] == x_size);

    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < x_size; ++j)
            REQUIRE(result[j][i] == values[j][columns[i]]);
}

TEMPLATE_LIST_TEST_CASE("columnSelection", "[template]", numerical_test_types) {
    columnSelectionTest<TestType>();
}

std::vector<std::array<size_t, 2>> global_indices_2d(const std::vector<size_t>& offset,
                                                     const std::vector<size_t>& count) {
    std::vector<std::array<size_t, 2>> indices;
    indices.reserve(count[0] * count[1]);

    for (size_t i = 0; i < count[0]; ++i) {
        for (size_t j = 0; j < count[1]; ++j) {
            indices.push_back({offset[0] + i, offset[1] + j});
        }
    }

    return indices;
}

std::vector<std::array<size_t, 2>> local_indices_2d(const std::vector<size_t>& count) {
    return global_indices_2d({0ul, 0ul}, count);
}

std::vector<std::array<size_t, 1>> local_indices_1d(const std::vector<size_t>& count) {
    std::vector<std::array<size_t, 1>> local_indices;
    for (size_t i = 0; i < count[0]; ++i) {
        local_indices.push_back({i});
    }

    return local_indices;
}

struct RegularHyperSlabAnswer {
    static RegularHyperSlabAnswer createRegular(const std::vector<size_t>& offset,
                                                const std::vector<size_t>& count) {
        return RegularHyperSlabAnswer{global_indices_2d(offset, count),
                                      local_indices_1d({count[0] * count[1]})};
    }

    // These are the selected indices in the
    // outer (larger) array.
    std::vector<std::array<size_t, 2>> global_indices;

    // These are the selected indices in the compacted (inner)
    // array.
    std::vector<std::array<size_t, 1>> local_indices;
};

struct RegularHyperSlabTestData {
    std::string desc;
    HyperSlab slab;
    RegularHyperSlabAnswer answer;
};

std::vector<RegularHyperSlabTestData> make_regular_hyperslab_test_data() {
    std::vector<RegularHyperSlabTestData> test_data;

    // The dataset is 10x8, we define the following regular
    // hyperslabs:
    //  x----------------x
    //  |                |
    //  | x------x   e   |  1
    //  | |  a   |       |
    //  x-|------|-------x  3
    //  | |    x-|-------x  4
    //  | |    | |  b    |
    //  | |    c-|-------c  5
    //  | |    b-|-------b  6
    //  | |    | |  c    |
    //  | d----x-d-------x  7
    //  | |  d   |       |
    //  | a------a       |  9
    //  |                |
    //  ------------------
    //    1    3 4       8

    std::map<std::string, RegularHyperSlab> slabs;

    slabs["a"] = RegularHyperSlab(/* offset = */ {1ul, 1ul},
                                  /* count = */ {8ul, 3ul});

    slabs["b"] = RegularHyperSlab(/* offset = */ {4ul, 3ul},
                                  /* count = */ {2ul, 5ul});

    slabs["c"] = RegularHyperSlab(/* offset = */ {5ul, 3ul},
                                  /* count = */ {2ul, 5ul});

    slabs["d"] = RegularHyperSlab(/* offset = */ {7ul, 1ul},
                                  /* count = */ {2ul, 3ul});

    slabs["e"] = RegularHyperSlab(/* offset = */ {0ul, 0ul},
                                  /* count = */ {3ul, 8ul});

    // Union, regular
    auto slab_bc_union = HyperSlab(slabs["b"]) | slabs["c"];
    auto answer_bc_union = RegularHyperSlabAnswer::createRegular({4ul, 3ul}, {3ul, 5ul});
    test_data.push_back({"b | c", slab_bc_union, answer_bc_union});

    // Intersection, always regular
    auto slab_ab_cut = HyperSlab(slabs["a"]) & slabs["b"];
    auto answer_ab_cut = RegularHyperSlabAnswer::createRegular({4ul, 3ul}, {2ul, 1ul});
    test_data.push_back({"a & b", slab_ab_cut, answer_ab_cut});

    // Intersection, always regular
    auto slab_bc_cut = HyperSlab(slabs["b"]) & slabs["c"];
    auto answer_bc_cut = RegularHyperSlabAnswer::createRegular({5ul, 3ul}, {1ul, 5ul});
    test_data.push_back({"b & c", slab_bc_cut, answer_bc_cut});

    // Xor, regular
    auto slab_ad_xor = HyperSlab(slabs["a"]) ^ slabs["d"];
    auto answer_ad_xor = RegularHyperSlabAnswer::createRegular({1ul, 1ul}, {6ul, 3ul});
    test_data.push_back({"a ^ b", slab_ad_xor, answer_ad_xor});

    // (not b) and c, regular
    auto slab_bc_nota = HyperSlab(slabs["b"]).notA(slabs["c"]);
    auto answer_bc_nota = RegularHyperSlabAnswer::createRegular({6ul, 3ul}, {1ul, 5ul});
    test_data.push_back({"b notA a", slab_bc_nota, answer_bc_nota});

    // (not c) and b, regular
    auto slab_cb_notb = HyperSlab(slabs["c"]).notB(slabs["b"]);
    auto answer_cb_notb = RegularHyperSlabAnswer::createRegular({6ul, 3ul}, {1ul, 5ul});
    test_data.push_back({"c notB b", slab_cb_notb, answer_cb_notb});

    return test_data;
}

template <class T, size_t x_size, size_t y_size>
File setupHyperSlabFile(T (&values)[x_size][y_size],
                        const std::string& filename,
                        const std::string& dataset_name) {
    ContentGenerate<T> generator;
    generate2D(values, x_size, y_size, generator);

    // Create a new file using the default property lists.
    File file(filename, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims{x_size, y_size};

    DataSpace dataspace(dims);
    // Create a dataset with arbitrary type
    DataSet dataset = file.createDataSet<T>(dataset_name, dataspace);

    dataset.write(values);
    file.flush();

    return file;
}

template <typename T>
void regularHyperSlabSelectionTest() {
    std::ostringstream filename;
    filename << "h5_rw_select_regular_hyperslab_test_" << typeNameHelper<T>() << "_test.h5";
    const std::string dataset_name("dset");

    const size_t x_size = 10;
    const size_t y_size = 8;

    T values[x_size][y_size];

    auto file = setupHyperSlabFile(values, filename.str(), dataset_name);
    auto test_cases = make_regular_hyperslab_test_data();

    for (const auto& test_case: test_cases) {
        SECTION(test_case.desc) {
            std::vector<T> result;

            file.getDataSet(dataset_name).select(test_case.slab).read(result);

            auto n_selected = test_case.answer.global_indices.size();
            for (size_t i = 0; i < n_selected; ++i) {
                const auto ig = test_case.answer.global_indices[i];
                const auto il = test_case.answer.local_indices[i];

                REQUIRE(result[il[0]] == values[ig[0]][ig[1]]);
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("hyperSlabSelection", "[template]", numerical_test_types) {
    regularHyperSlabSelectionTest<TestType>();
}

struct IrregularHyperSlabAnswer {
    // These are the selected indices in the outer (larger) array.
    std::vector<std::array<size_t, 2>> global_indices;
};

struct IrregularHyperSlabTestData {
    std::string desc;
    HyperSlab slab;
    IrregularHyperSlabAnswer answer;
};

std::vector<IrregularHyperSlabTestData> make_irregular_hyperslab_test_data() {
    // The dataset is 10x8, with two regular hyperslabs:
    //  x----------------x
    //  |                |
    //  |    bbbb        |
    //  |    bbbb        |
    //  |  aaaabb        |
    //  |  aaaabb        |
    //  |    bbbb        |
    //  |    bbbb        |
    //  |                |
    //  |                |
    //  |                |
    //  |                |
    //  ------------------

    auto slabs = std::map<std::string, RegularHyperSlab>{};
    slabs["a"] = RegularHyperSlab{{2ul, 0ul}, {1ul, 2ul}};
    slabs["b"] = RegularHyperSlab{{1ul, 1ul}, {3ul, 2ul}};

    std::vector<IrregularHyperSlabTestData> test_data;

    // Union, irregular
    auto slab_ab_union = HyperSlab(slabs["a"]) | slabs["b"];
    // clang-format off
    auto answer_ab_union = IrregularHyperSlabAnswer{{
                    {1ul, 1ul}, {1ul, 2ul},
        {2ul, 0ul}, {2ul, 1ul}, {2ul, 2ul},
                    {3ul, 1ul}, {3ul, 2ul}
    }};
    // clang-format on
    test_data.push_back({"a | b", slab_ab_union, answer_ab_union});

    // xor, irregular
    auto slab_ab_xor = HyperSlab(slabs["a"]) ^ slabs["b"];
    // clang-format off
    auto answer_ab_xor = IrregularHyperSlabAnswer{{
                    {1ul, 1ul}, {1ul, 2ul},
        {2ul, 0ul},             {2ul, 2ul},
                    {3ul, 1ul}, {3ul, 2ul}
    }};
    // clang-format on
    test_data.push_back({"a xor b", slab_ab_xor, answer_ab_xor});

    // (not a) and e, irregular
    auto slab_ab_nota = HyperSlab(slabs["a"]).notA(slabs["b"]);
    // clang-format off
    auto answer_ab_nota = IrregularHyperSlabAnswer{{
                    {1ul, 1ul}, {1ul, 2ul},
                                {2ul, 2ul},
                    {3ul, 1ul}, {3ul, 2ul}
    }};
    // clang-format on
    test_data.push_back({"a nota b", slab_ab_nota, answer_ab_nota});

    // (not a) and e, irregular
    auto slab_ba_notb = HyperSlab(slabs["b"]).notB(slabs["a"]);
    // clang-format off
    auto answer_ba_notb = IrregularHyperSlabAnswer{{
                     {1ul, 1ul}, {1ul, 2ul},
                                 {2ul, 2ul},
                     {3ul, 1ul}, {3ul, 2ul}
    }};
    // clang-format on
    test_data.push_back({"b notb a", slab_ba_notb, answer_ba_notb});

    return test_data;
}

template <typename T>
void irregularHyperSlabSelectionReadTest() {
    std::ostringstream filename;
    filename << "h5_write_select_irregular_hyperslab_test_" << typeNameHelper<T>() << "_test.h5";

    const std::string dataset_name("dset");

    const size_t x_size = 10;
    const size_t y_size = 8;

    T values[x_size][y_size];
    auto file = setupHyperSlabFile(values, filename.str(), dataset_name);

    auto test_cases = make_irregular_hyperslab_test_data();

    for (const auto& test_case: test_cases) {
        SECTION(test_case.desc) {
            std::vector<T> result;

            file.getDataSet(dataset_name).select(test_case.slab).read(result);

            auto n_selected = test_case.answer.global_indices.size();
            for (size_t i = 0; i < n_selected; ++i) {
                const auto ig = test_case.answer.global_indices[i];

                REQUIRE(result[i] == values[ig[0]][ig[1]]);
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("irregularHyperSlabSelectionRead", "[template]", numerical_test_types) {
    irregularHyperSlabSelectionReadTest<TestType>();
}

template <typename T>
void irregularHyperSlabSelectionWriteTest() {
    std::ostringstream filename;
    filename << "h5_write_select_irregular_hyperslab_test_" << typeNameHelper<T>() << "_test.h5";

    const std::string dataset_name("dset");

    const size_t x_size = 10;
    const size_t y_size = 8;

    T orig_values[x_size][y_size];
    auto file = setupHyperSlabFile(orig_values, filename.str(), dataset_name);

    auto test_cases = make_irregular_hyperslab_test_data();

    for (const auto& test_case: test_cases) {
        SECTION(test_case.desc) {
            auto n_selected = test_case.answer.global_indices.size();
            std::vector<T> changed_values(n_selected);
            ContentGenerate<T> gen;
            std::generate(changed_values.begin(), changed_values.end(), gen);

            file.getDataSet(dataset_name).select(test_case.slab).write(changed_values);

            T overwritten_values[x_size][y_size];
            file.getDataSet(dataset_name).read(overwritten_values);

            T expected_values[x_size][y_size];
            for (size_t i = 0; i < x_size; ++i) {
                for (size_t j = 0; j < y_size; ++j) {
                    expected_values[i][j] = orig_values[i][j];
                }
            }

            for (size_t i = 0; i < n_selected; ++i) {
                const auto ig = test_case.answer.global_indices[i];
                expected_values[ig[0]][ig[1]] = changed_values[i];
            }

            for (size_t i = 0; i < x_size; ++i) {
                for (size_t j = 0; j < y_size; ++j) {
                    REQUIRE(expected_values[i][j] == overwritten_values[i][j]);
                }
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("irregularHyperSlabSelectionWrite", "[template]", std::tuple<int>) {
    irregularHyperSlabSelectionWriteTest<TestType>();
}

void check_selected(const std::vector<int>& selected,
                    const std::vector<std::array<size_t, 2>>& indices,
                    const std::vector<std::vector<int>>& x) {
    REQUIRE(selected.size() == indices.size());
    for (size_t k = 0; k < selected.size(); ++k) {
        size_t i = indices[k][0];
        size_t j = indices[k][1];
        REQUIRE(selected[k] == x[i][j]);
    }
}

TEST_CASE("select_multiple_ors", "[hyperslab]") {
    size_t n = 100, m = 20;
    size_t nsel = 30;
    auto x = testing::DataGenerator<std::vector<std::vector<int>>>::create({n, m});

    auto file = File("select_multiple_ors.h5", File::Truncate);
    auto dset = file.createDataSet("x", x);

    std::vector<std::array<size_t, 2>> indices;
    auto hyperslab = HyperSlab();
    for (size_t i = 0; i < nsel; ++i) {
        std::vector<size_t> offsets{i, i % 10};
        std::vector<size_t> counts{1, 3};
        hyperslab |= RegularHyperSlab(offsets, counts);

        for (size_t k = 0; k < counts[1]; ++k) {
            indices.push_back({offsets[0], offsets[1] + k});
        }
    }

    SECTION("Pure Or Chain") {
        auto selected = dset.select(hyperslab).read<std::vector<int>>();
        check_selected(selected, indices, x);
    }

    SECTION("Or Chain And Slab") {
        std::vector<size_t> offsets{5, 2};
        std::vector<size_t> counts{85, 12};

        std::vector<std::array<size_t, 2>> selected_indices;
        for (const auto ij: indices) {
            std::array<size_t, 2> ij_max = {offsets[0] + counts[0], offsets[1] + counts[1]};

            if (offsets[0] <= ij[0] && ij[0] < ij_max[0] && offsets[1] <= ij[1] &&
                ij[1] < ij_max[1]) {
                selected_indices.push_back(ij);
            }
        }

        hyperslab &= RegularHyperSlab(offsets, counts);

        auto selected = dset.select(hyperslab).read<std::vector<int>>();
        check_selected(selected, selected_indices, x);
    }
}

TEST_CASE("select_multiple_ors_edge_cases", "[hyperslab]") {
    size_t n = 100, m = 20;

    auto x = testing::DataGenerator<std::vector<std::vector<int>>>::create({n, m});
    auto file = File("select_multiple_ors_edge_cases.h5", File::Truncate);
    auto dset = file.createDataSet("x", x);

    std::vector<std::array<size_t, 2>> indices;
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            indices.push_back({i, j});
        }
    }

    auto space = DataSpace({n, m});
    SECTION("complete |= ") {
        auto hyperslab = HyperSlab(RegularHyperSlab({0, 0}, {n, m}));

        // Select everything; and then redundantly some parts again.
        hyperslab &= RegularHyperSlab({0, 0}, {n, m});
        hyperslab |= RegularHyperSlab({0, 0}, {n, m / 2});
        hyperslab |= RegularHyperSlab({3, 0}, {1, 3});
        hyperslab |= RegularHyperSlab({6, 0}, {1, 3});
        hyperslab.apply(space);

        auto selected = dset.select(hyperslab).read<std::vector<int>>();
        check_selected(selected, indices, x);
    }

    SECTION("complete &=") {
        auto hyperslab = HyperSlab();

        // With detours, select everything, then reduce it to the first 2
        // elements.
        hyperslab |= RegularHyperSlab({0, 0}, {n, m / 2});
        hyperslab |= RegularHyperSlab({0, 0}, {n, m / 2});
        hyperslab |= RegularHyperSlab({0, m / 2}, {n, m - m / 2});
        hyperslab |= RegularHyperSlab({0, 0}, {n, m});
        hyperslab &= RegularHyperSlab({0, 0}, {1, 2});
        hyperslab.apply(space);

        indices = {{0, 0}, {0, 1}};

        auto selected = dset.select(hyperslab).read<std::vector<int>>();
        check_selected(selected, indices, x);
    }

    SECTION("empty |=") {
        auto hyperslab = HyperSlab(RegularHyperSlab({0, 0}, {n, m}));
        hyperslab &= RegularHyperSlab({0, 0}, {1, 2});
        hyperslab &= RegularHyperSlab({3, 0}, {1, 2});

        hyperslab |= RegularHyperSlab({0, 0}, {n, m / 2});
        hyperslab |= RegularHyperSlab({0, 0}, {n, m / 2});
        hyperslab |= RegularHyperSlab({0, m / 2}, {n, m - m / 2});
        hyperslab |= RegularHyperSlab({0, 0}, {n, m});

        hyperslab.apply(space);

        auto selected = dset.select(hyperslab).read<std::vector<int>>();
        check_selected(selected, indices, x);
    }

    SECTION("|= empty") {
        auto hyperslab = HyperSlab();

        hyperslab |= RegularHyperSlab({0, 0}, {1, 2});
        hyperslab |= RegularHyperSlab({0, 0}, {1, 2});
        hyperslab |= RegularHyperSlab({0, 0}, {0, 0});

        hyperslab.apply(space);

        indices = {{0, 0}, {0, 1}};

        auto selected = dset.select(hyperslab).read<std::vector<int>>();
        check_selected(selected, indices, x);
    }
}
