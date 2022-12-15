/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
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
#include <vector>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Reference.hpp>
#include <highfive/H5Utility.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;
using Catch::Matchers::Equals;

TEST_CASE("Basic HighFive tests") {
    const std::string FILE_NAME("h5tutr_dset.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    CHECK(file.getName() == FILE_NAME);

    // Create the data space for the dataset.
    std::vector<size_t> dims{4, 6};

    DataSpace dataspace(dims);

    // check if the dataset exist
    CHECK(!file.exist(DATASET_NAME + "_double"));

    // Create a dataset with double precision floating points
    DataSet dataset_double =
        file.createDataSet(DATASET_NAME + "_double", dataspace, AtomicType<double>());

    CHECK(file.getObjectName(0) == DATASET_NAME + "_double");

    {
        // check if it exist again
        CHECK(file.exist(DATASET_NAME + "_double"));

        // and also try to recreate it to the sake of exception testing
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.createDataSet(DATASET_NAME + "_double",
                                           dataspace,
                                           AtomicType<double>()),
                        DataSetException);
    }

    DataSet dataset_size_t = file.createDataSet<size_t>(DATASET_NAME + "_size_t", dataspace);
}

TEST_CASE("Test silent HighFive") {
    // Setting up a buffer for stderr so we can detect if the stack trace
    // was disabled
    fflush(stderr);
    char buffer[1024];
    memset(buffer, 0, sizeof(char) * 1024);
    setvbuf(stderr, buffer, _IOLBF, 1023);

    try {
        SilenceHDF5 silence;
        File file("nonexistent", File::ReadOnly);
    } catch (const FileException&) {
    }
    CHECK(buffer[0] == '\0');

    // restore the dyn allocated buffer
    // or using stderr will segfault when buffer get out of scope
    fflush(stderr);
    setvbuf(stderr, NULL, _IONBF, 0);
}

TEST_CASE("Test open modes in HighFive") {
    const std::string FILE_NAME("openmodes.h5");

    std::remove(FILE_NAME.c_str());

    SilenceHDF5 silencer;

    // Attempt open file only ReadWrite should fail (wont create)
    CHECK_THROWS_AS(File(FILE_NAME, File::ReadWrite), FileException);

    // But with Create flag should be fine
    { File file(FILE_NAME, File::ReadWrite | File::Create); }

    // But if its there and exclusive is given, should fail
    CHECK_THROWS_AS(File(FILE_NAME, File::ReadWrite | File::Excl), FileException);
    // ReadWrite and Excl flags are fine together (posix)
    std::remove(FILE_NAME.c_str());
    { File file(FILE_NAME, File::ReadWrite | File::Excl); }
    // All three are fine as well (as long as the file does not exist)
    std::remove(FILE_NAME.c_str());
    { File file(FILE_NAME, File::ReadWrite | File::Create | File::Excl); }

    // Just a few combinations are incompatible, detected by hdf5lib
    CHECK_THROWS_AS(File(FILE_NAME, File::Truncate | File::Excl), FileException);

    std::remove(FILE_NAME.c_str());
    CHECK_THROWS_AS(File(FILE_NAME, File::Truncate | File::Excl), FileException);

    // But in most cases we will truncate and that should always work
    { File file(FILE_NAME, File::Truncate); }
    std::remove(FILE_NAME.c_str());
    { File file(FILE_NAME, File::Truncate); }

    // Last but not least, defaults should be ok
    { File file(FILE_NAME); }     // ReadOnly
    { File file(FILE_NAME, 0); }  // force empty-flags, does open without flags
}

TEST_CASE("Test file version bounds") {
    const std::string FILE_NAME("h5_version_bounds.h5");

    std::remove(FILE_NAME.c_str());

    {
        File file(FILE_NAME, File::Truncate);
        auto bounds = file.getVersionBounds();
        CHECK(bounds.first == H5F_LIBVER_EARLIEST);
        CHECK(bounds.second == H5F_LIBVER_LATEST);
    }

    std::remove(FILE_NAME.c_str());

    {
        FileAccessProps fapl;
        fapl.add(FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
        File file(FILE_NAME, File::Truncate, fapl);
        auto bounds = file.getVersionBounds();
        CHECK(bounds.first == H5F_LIBVER_LATEST);
        CHECK(bounds.second == H5F_LIBVER_LATEST);
    }
}

#if H5_VERSION_GE(1, 10, 1)
TEST_CASE("Test file space strategy") {
    const std::string FILE_NAME("h5_file_space_strategy.h5");
    auto strategies = std::vector<H5F_fspace_strategy_t>{H5F_FSPACE_STRATEGY_FSM_AGGR,
                                                         H5F_FSPACE_STRATEGY_AGGR,
                                                         H5F_FSPACE_STRATEGY_PAGE,
                                                         H5F_FSPACE_STRATEGY_NONE};

    for (const auto& strategy: strategies) {
        {
            FileCreateProps create_props;
            create_props.add(FileSpaceStrategy(strategy, true, 0));

            File file(FILE_NAME, File::Truncate, create_props);
        }

        {
            File file(FILE_NAME, File::ReadOnly);
            CHECK(file.getFileSpaceStrategy() == strategy);
        }
    }
}

TEST_CASE("Test file space page size") {
    const std::string FILE_NAME("h5_file_space_page_size.h5");
    hsize_t page_size = 1024;
    {
        FileCreateProps create_props;
        create_props.add(FileSpaceStrategy(H5F_FSPACE_STRATEGY_PAGE, true, 0));
        create_props.add(FileSpacePageSize(page_size));

        File file(FILE_NAME, File::Truncate, create_props);
    }

    {
        File file(FILE_NAME, File::ReadOnly);
        CHECK(file.getFileSpacePageSize() == page_size);
    }
}

#ifndef H5_HAVE_PARALLEL
TEST_CASE("Test page buffer size") {
    const std::string FILE_NAME("h5_page_buffer_size.h5");
    hsize_t page_size = 1024;
    {
        FileCreateProps create_props;
        create_props.add(FileSpaceStrategy(H5F_FSPACE_STRATEGY_PAGE, true, 0));
        create_props.add(FileSpacePageSize(page_size));

        File file(FILE_NAME, File::Truncate, create_props);

        file.createDataSet("x", std::vector<double>{1.0, 2.0, 3.0});
    }

    {
        FileAccessProps access_props;
        access_props.add(PageBufferSize(1024));

        File file(FILE_NAME, File::ReadOnly, access_props);

        auto accesses = std::array<unsigned int, 2>{0, 0};
        auto hits = std::array<unsigned int, 2>{0, 0};
        auto misses = std::array<unsigned int, 2>{0, 0};
        auto evictions = std::array<unsigned int, 2>{0, 0};
        auto bypasses = std::array<unsigned int, 2>{0, 0};

        auto err = H5Fget_page_buffering_stats(file.getId(),
                                               accesses.data(),
                                               hits.data(),
                                               misses.data(),
                                               evictions.data(),
                                               bypasses.data());
        REQUIRE(err >= 0);

        CHECK(accesses[0] == 0);
        CHECK(accesses[1] == 0);

        CHECK(bypasses[0] == 0);
        CHECK(bypasses[1] == 0);

        auto x = file.getDataSet("x").read<std::vector<double>>();

        err = H5Fget_page_buffering_stats(file.getId(),
                                          accesses.data(),
                                          hits.data(),
                                          misses.data(),
                                          evictions.data(),
                                          bypasses.data());
        REQUIRE(err >= 0);

        CHECK(accesses[0] > 0);
        CHECK(accesses[1] == 1);

        CHECK(bypasses[0] == 0);
        CHECK(bypasses[1] == 0);

        CHECK(file.getFileSpacePageSize() == page_size);
    }
}
#endif
#endif

TEST_CASE("Test metadata block size assignment") {
    const std::string FILE_NAME("h5_meta_block_size.h5");

    std::remove(FILE_NAME.c_str());

    {
        File file(FILE_NAME, File::Truncate);
        // Default for HDF5
        CHECK(file.getMetadataBlockSize() == 2048);
    }

    std::remove(FILE_NAME.c_str());

    {
        FileAccessProps fapl;
        fapl.add(MetadataBlockSize(10240));
        File file(FILE_NAME, File::Truncate, fapl);
        CHECK(file.getMetadataBlockSize() == 10240);
    }
}

TEST_CASE("Test group properties") {
    const std::string FILE_NAME("h5_group_properties.h5");
    FileAccessProps fapl;
    // When using hdf5 1.10.2 and later, the lower bound may be set to
    // H5F_LIBVER_V18
    fapl.add(FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    File file(FILE_NAME, File::Truncate, fapl);

    GroupCreateProps props;
    props.add(EstimatedLinkInfo(1000, 500));
    auto group = file.createGroup("g", props);
    auto sizes = group.getEstimatedLinkInfo();

    CHECK(sizes.first == 1000);
    CHECK(sizes.second == 500);
}

TEST_CASE("Test allocation time") {
    const std::string FILE_NAME("h5_dataset_alloc_time.h5");
    File file(FILE_NAME, File::Truncate);

    size_t n_elements = 10;
    std::vector<double> data(n_elements);

    auto dcpl = DataSetCreateProps{};
    dcpl.add(AllocationTime(H5D_ALLOC_TIME_EARLY));

    auto dataspace = DataSpace::From(data);
    auto datatype = create_datatype<decltype(data)::value_type>();
    auto dataset = file.createDataSet("dset", dataspace, datatype, dcpl);

    auto alloc_size = H5Dget_storage_size(dataset.getId());
    CHECK(alloc_size == data.size() * sizeof(decltype(data)::value_type));
}

TEST_CASE("Test default constructors") {
    const std::string FILE_NAME("h5_group_test.h5");
    const std::string DATASET_NAME("dset");
    File file(FILE_NAME, File::Truncate);
    auto ds = file.createDataSet(DATASET_NAME, std::vector<int>{1, 2, 3, 4, 5});

    DataSet d2;  // deprecated as it constructs unsafe objects
    // d2.getFile();  // runtime error
    CHECK(!d2.isValid());
    d2 = ds;  // copy
    CHECK(d2.isValid());
}

TEST_CASE("Test groups and datasets") {
    const std::string FILE_NAME("h5_group_test.h5");
    const std::string DATASET_NAME("dset");
    const std::string CHUNKED_DATASET_NAME("chunked_dset");
    const std::string CHUNKED_DATASET_SMALL_NAME("chunked_dset_small");
    const std::string GROUP_NAME1("/group1");
    const std::string GROUP_NAME2("group2");
    const std::string GROUP_NESTED_NAME("group_nested");

    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // absolute group
        file.createGroup(GROUP_NAME1);
        // nested group absolute
        file.createGroup(GROUP_NAME1 + "/" + GROUP_NESTED_NAME);
        // relative group
        Group g1 = file.createGroup(GROUP_NAME2);
        // relative group
        Group nested = g1.createGroup(GROUP_NESTED_NAME);

        // Create the data space for the dataset.
        std::vector<size_t> dims{4, 6};

        DataSpace dataspace(dims);

        DataSet dataset_absolute = file.createDataSet(GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" +
                                                          DATASET_NAME,
                                                      dataspace,
                                                      AtomicType<double>());

        DataSet dataset_relative =
            nested.createDataSet(DATASET_NAME, dataspace, AtomicType<double>());

        DataSetCreateProps goodChunking;
        goodChunking.add(Chunking(std::vector<hsize_t>{2, 2}));
        DataSetAccessProps cacheConfig;
        cacheConfig.add(Caching(13, 1024, 0.5));

        // will fail because exceeds dimensions
        DataSetCreateProps badChunking0;
        badChunking0.add(Chunking(std::vector<hsize_t>{10, 10}));

        DataSetCreateProps badChunking1;
        badChunking1.add(Chunking(std::vector<hsize_t>{1, 1, 1}));

        {
            SilenceHDF5 silencer;
            CHECK_THROWS_AS(file.createDataSet(CHUNKED_DATASET_NAME,
                                               dataspace,
                                               AtomicType<double>(),
                                               badChunking0),
                            DataSetException);

            CHECK_THROWS_AS(file.createDataSet(CHUNKED_DATASET_NAME,
                                               dataspace,
                                               AtomicType<double>(),
                                               badChunking1),
                            DataSetException);
        }

        // here we use the other signature
        DataSet dataset_chunked =
            file.createDataSet<float>(CHUNKED_DATASET_NAME, dataspace, goodChunking, cacheConfig);

        // Here we resize to smaller than the chunking size
        DataSet dataset_chunked_small =
            file.createDataSet<float>(CHUNKED_DATASET_SMALL_NAME, dataspace, goodChunking);

        dataset_chunked_small.resize({1, 1});
    }
    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);
        Group g1 = file.getGroup(GROUP_NAME1);
        Group g2 = file.getGroup(GROUP_NAME2);
        Group nested_group2 = g2.getGroup(GROUP_NESTED_NAME);

        DataSet dataset_absolute = file.getDataSet(GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" +
                                                   DATASET_NAME);
        CHECK(4 == dataset_absolute.getSpace().getDimensions()[0]);

        DataSet dataset_relative = nested_group2.getDataSet(DATASET_NAME);
        CHECK(4 == dataset_relative.getSpace().getDimensions()[0]);

        DataSetAccessProps accessProps;
        accessProps.add(Caching(13, 1024, 0.5));
        DataSet dataset_chunked = file.getDataSet(CHUNKED_DATASET_NAME, accessProps);
        CHECK(4 == dataset_chunked.getSpace().getDimensions()[0]);

        DataSet dataset_chunked_small = file.getDataSet(CHUNKED_DATASET_SMALL_NAME);
        CHECK(1 == dataset_chunked_small.getSpace().getDimensions()[0]);
    }
}

TEST_CASE("Test extensible datasets") {
    const std::string FILE_NAME("create_extensible_dataset_example.h5");
    const std::string DATASET_NAME("dset");
    constexpr long double t1[3][1] = {{2.0l}, {2.0l}, {4.0l}};
    constexpr long double t2[1][3] = {{4.0l, 8.0l, 6.0l}};

    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create a dataspace with initial shape and max shape
        DataSpace dataspace = DataSpace({4, 5}, {17, DataSpace::UNLIMITED});

        // Use chunking
        DataSetCreateProps props;
        props.add(Chunking(std::vector<hsize_t>{2, 2}));

        // Create the dataset
        DataSet dataset =
            file.createDataSet(DATASET_NAME, dataspace, AtomicType<long double>(), props);

        // Write into the initial part of the dataset
        dataset.select({0, 0}, {3, 1}).write(t1);

        // Resize the dataset to a larger size
        dataset.resize({4, 6});

        CHECK(4 == dataset.getSpace().getDimensions()[0]);
        CHECK(6 == dataset.getSpace().getDimensions()[1]);

        // Write into the new part of the dataset
        dataset.select({3, 3}, {1, 3}).write(t2);

        SilenceHDF5 silencer;
        // Try resize out of bounds
        CHECK_THROWS_AS(dataset.resize({18, 1}), DataSetException);
        // Try resize invalid dimensions
        CHECK_THROWS_AS(dataset.resize({1, 2, 3}), DataSetException);
    }

    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);

        DataSet dataset_absolute = file.getDataSet("/" + DATASET_NAME);
        const auto dims = dataset_absolute.getSpace().getDimensions();
        long double values[4][6];
        dataset_absolute.read(values);
        CHECK(4 == dims[0]);
        CHECK(6 == dims[1]);

        CHECK(t1[0][0] == values[0][0]);
        CHECK(t1[1][0] == values[1][0]);
        CHECK(t1[2][0] == values[2][0]);

        CHECK(t2[0][0] == values[3][3]);
        CHECK(t2[0][1] == values[3][4]);
        CHECK(t2[0][2] == values[3][5]);
    }
}

TEST_CASE("Test reference count") {
    const std::string FILE_NAME("h5_ref_count_test.h5");
    const std::string DATASET_NAME("dset");
    const std::string GROUP_NAME1("/group1");
    const std::string GROUP_NAME2("/group2");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    std::unique_ptr<DataSet> d1_ptr;
    std::unique_ptr<Group> g_ptr;

    {
        // create group
        Group g1 = file.createGroup(GROUP_NAME1);

        // override object
        g1 = file.createGroup(GROUP_NAME2);

        // Create the data space for the dataset.
        std::vector<size_t> dims = {10, 10};

        DataSpace dataspace(dims);

        DataSet d1 =
            file.createDataSet(GROUP_NAME1 + DATASET_NAME, dataspace, AtomicType<double>());

        double values[10][10] = {{0}};
        values[5][0] = 1;
        d1.write(values);

        // force move
        d1_ptr.reset(new DataSet(std::move(d1)));

        // force copy
        g_ptr.reset(new Group(g1));
    }
    // read it back
    {
        DataSet d2(std::move(*d1_ptr));
        d1_ptr.reset();

        double values[10][10];
        d2.read(values);

        for (std::size_t i = 0; i < 10; ++i) {
            for (std::size_t j = 0; j < 10; ++j) {
                double v = values[i][j];

                if (i == 5 && j == 0) {
                    REQUIRE(v == 1);
                } else {
                    REQUIRE(v == 0);
                }
            }
        }

        // force copy
        Group g2 = *g_ptr;

        // add a subgroup
        g2.createGroup("blabla");
    }
}

TEST_CASE("Test simple listings") {
    const std::string FILE_NAME("h5_list_test.h5");
    const std::string GROUP_NAME_CORE("group_name");
    const std::string GROUP_NESTED_NAME("/group_nested");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    {
        // absolute group
        for (int i = 0; i < 2; ++i) {
            std::ostringstream ss;
            ss << "/" << GROUP_NAME_CORE << "_" << i;
            file.createGroup(ss.str());
        }

        size_t n_elem = file.getNumberObjects();
        CHECK(2 == n_elem);

        std::vector<std::string> elems = file.listObjectNames();
        CHECK(2 == elems.size());
        std::vector<std::string> reference_elems;
        for (int i = 0; i < 2; ++i) {
            std::ostringstream ss;
            ss << GROUP_NAME_CORE << "_" << i;
            reference_elems.push_back(ss.str());
        }

        CHECK(elems == reference_elems);
    }

    {
        file.createGroup(GROUP_NESTED_NAME);
        Group g_nest = file.getGroup(GROUP_NESTED_NAME);

        for (int i = 0; i < 50; ++i) {
            std::ostringstream ss;
            ss << GROUP_NAME_CORE << "_" << i;
            g_nest.createGroup(ss.str());
        }

        size_t n_elem = g_nest.getNumberObjects();
        CHECK(50 == n_elem);

        std::vector<std::string> elems = g_nest.listObjectNames();
        CHECK(50 == elems.size());
        std::vector<std::string> reference_elems;

        for (int i = 0; i < 50; ++i) {
            std::ostringstream ss;
            ss << GROUP_NAME_CORE << "_" << i;
            reference_elems.push_back(ss.str());
        }
        // there is no guarantee on the order of the hdf5 index, let's sort it
        // to put them in order
        std::sort(elems.begin(), elems.end());
        std::sort(reference_elems.begin(), reference_elems.end());

        CHECK(elems == reference_elems);
    }
}

TEST_CASE("Simple test for type equality") {
    AtomicType<double> d_var;
    AtomicType<size_t> size_var;
    AtomicType<double> d_var_test;
    AtomicType<size_t> size_var_cpy(size_var);
    AtomicType<int> int_var;
    AtomicType<unsigned> uint_var;

    // check different type matching
    CHECK(d_var == d_var_test);
    CHECK(d_var != size_var);

    // check type copy matching
    CHECK(size_var_cpy == size_var);

    // check sign change not matching
    CHECK(int_var != uint_var);
}

TEST_CASE("DataTypeEqualTakeBack") {
    const std::string FILE_NAME("h5tutr_dset.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims{10, 1};

    DataSpace dataspace(dims);

    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet<size_t>(DATASET_NAME + "_double", dataspace);

    AtomicType<size_t> s;
    AtomicType<double> d;

    CHECK(s == dataset.getDataType());
    CHECK(d != dataset.getDataType());

    // Test getAddress and expect deprecation warning
    auto addr = dataset.getInfo().getAddress();
    CHECK(addr != 0);
}

TEST_CASE("DataSpaceTest") {
    const std::string FILE_NAME("h5tutr_space.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    DataSpace dataspace{std::vector<size_t>{10, 1}};

    // Create a dataset with size_t type
    DataSet dataset = file.createDataSet<size_t>(DATASET_NAME, dataspace);

    DataSpace space = dataset.getSpace();
    DataSpace space2 = dataset.getSpace();

    // verify space id are different
    CHECK(space.getId() != space2.getId());

    // verify space id are consistent
    CHECK(space.getDimensions().size() == 2);
    CHECK(space.getDimensions()[0] == 10);
    CHECK(space.getDimensions()[1] == 1);
}

TEST_CASE("DataSpaceVectorTest") {
    // Create 1D shortcut dataspace
    DataSpace space(7);

    CHECK(space.getDimensions().size() == 1);
    CHECK(space.getDimensions()[0] == 7);

    // Initializer list (explicit)
    DataSpace space3({8, 9, 10});
    auto space3_res = space3.getDimensions();
    std::vector<size_t> space3_ans{8, 9, 10};

    CHECK(space3_res == space3_ans);

    // Verify 2D works (note that without the {}, this matches the iterator
    // constructor)
    DataSpace space2(std::vector<size_t>{3, 4});

    auto space2_res = space2.getDimensions();
    std::vector<size_t> space2_ans{3, 4};

    CHECK(space2_res == space2_ans);
}

TEST_CASE("DataSpaceVariadicTest") {
    // Create 1D shortcut dataspace
    DataSpace space1{7};

    auto space1_res = space1.getDimensions();
    std::vector<size_t> space1_ans{7};

    CHECK(space1_res == space1_ans);

    // Initializer list (explicit)
    DataSpace space3{8, 9, 10};

    auto space3_res = space3.getDimensions();
    std::vector<size_t> space3_ans{8, 9, 10};

    CHECK(space3_res == space3_ans);

    // Verify 2D works using explicit syntax
    DataSpace space2{3, 4};

    auto space2_res = space2.getDimensions();
    std::vector<size_t> space2_ans{3, 4};

    CHECK(space2_res == space2_ans);

    // Verify 2D works using old syntax (this used to match the iterator!)
    DataSpace space2b(3, 4);

    auto space2b_res = space2b.getDimensions();
    std::vector<size_t> space2b_ans{3, 4};

    CHECK(space2b_res == space2b_ans);
}

TEST_CASE("ChunkingConstructorsTest") {
    Chunking first(1, 2, 3);

    auto first_res = first.getDimensions();
    std::vector<hsize_t> first_ans{1, 2, 3};

    CHECK(first_res == first_ans);

    Chunking second{1, 2, 3};

    auto second_res = second.getDimensions();
    std::vector<hsize_t> second_ans{1, 2, 3};

    CHECK(second_res == second_ans);

    Chunking third({1, 2, 3});

    auto third_res = third.getDimensions();
    std::vector<hsize_t> third_ans{1, 2, 3};

    CHECK(third_res == third_ans);
}

TEST_CASE("HighFiveReadWriteShortcut") {
    std::ostringstream filename;
    filename << "h5_rw_vec_shortcut_test.h5";

    const unsigned x_size = 800;
    const std::string DATASET_NAME("dset");
    std::vector<unsigned> vec;
    vec.resize(x_size);
    for (unsigned i = 0; i < x_size; i++)
        vec[i] = i * 2;
    std::string at_contents("Contents of string");
    int my_int = 3;
    std::vector<std::vector<int>> my_nested = {{1, 2}, {3, 4}};

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    // Create a dataset with int points
    DataSet dataset = file.createDataSet(DATASET_NAME, vec);
    dataset.createAttribute("str", at_contents);

    DataSet ds_int = file.createDataSet("/TmpInt", my_int);
    DataSet ds_nested = file.createDataSet("/TmpNest", my_nested);

    std::vector<unsigned> result;
    dataset.read(result);
    CHECK_THAT(vec, Equals(result));

    std::string read_in;
    dataset.getAttribute("str").read(read_in);
    CHECK(read_in == at_contents);

    int out_int = 0;
    ds_int.read(out_int);
    CHECK(my_int == out_int);

    decltype(my_nested) out_nested;
    ds_nested.read(out_nested);

    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            REQUIRE(my_nested[i][j] == out_nested[i][j]);
        }
    }

    // Plain c arrays. 1D
    {
        int int_c_array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        DataSet ds_int2 = file.createDataSet("/TmpCArrayInt", int_c_array);

        decltype(int_c_array) int_c_array_out;
        ds_int2.read(int_c_array_out);
        for (size_t i = 0; i < 10; ++i) {
            REQUIRE(int_c_array[i] == int_c_array_out[i]);
        }
    }

    // Plain c arrays. 2D
    {
        char char_c_2darray[][3] = {"aa", "bb", "cc", "12"};
        DataSet ds_char2 = file.createDataSet("/TmpCArray2dchar", char_c_2darray);

        decltype(char_c_2darray) char_c_2darray_out;
        ds_char2.read(char_c_2darray_out);
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                REQUIRE(char_c_2darray[i][j] == char_c_2darray_out[i][j]);
            }
        }
    }
}

template <typename T>
void readWriteAttributeVectorTest() {
    std::ostringstream filename;
    filename << "h5_rw_attribute_vec_" << typeNameHelper<T>() << "_test.h5";

    std::srand((unsigned) std::time(0));
    const size_t x_size = 25;
    const std::string DATASET_NAME("dset");
    typename std::vector<T> vec;

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    vec.resize(x_size);
    ContentGenerate<T> generator;
    std::generate(vec.begin(), vec.end(), generator);

    {
        // Create a dummy group to annotate with an attribute
        Group g = file.createGroup("dummy_group");

        // check that no attributes are there
        std::size_t n = g.getNumberAttributes();
        CHECK(n == 0);

        std::vector<std::string> all_attribute_names = g.listAttributeNames();
        CHECK(all_attribute_names.size() == 0);
        CHECK(!g.hasAttribute("my_attribute"));

        Attribute a1 = g.createAttribute<T>("my_attribute", DataSpace::From(vec));
        a1.write(vec);

        // check now that we effectively have an attribute listable
        CHECK(g.getNumberAttributes() == 1);
        CHECK(g.hasAttribute("my_attribute"));

        all_attribute_names = g.listAttributeNames();
        CHECK(all_attribute_names.size() == 1);
        CHECK(all_attribute_names[0] == std::string("my_attribute"));

        // Create the same attribute on a newly created dataset
        DataSet s = g.createDataSet("dummy_dataset", DataSpace(1), AtomicType<int>());

        Attribute a2 = s.createAttribute<T>("my_attribute_copy", DataSpace::From(vec));
        a2.write(vec);

        // const data, short-circuit syntax
        const std::vector<int> v{1, 2, 3};
        s.createAttribute("version_test", v);
    }

    {
        typename std::vector<T> result1, result2;

        Attribute a1_read = file.getGroup("dummy_group").getAttribute("my_attribute");
        a1_read.read(result1);

        CHECK(vec.size() == x_size);
        CHECK(result1.size() == x_size);
        CHECK(vec == result1);

        Attribute a2_read =
            file.getDataSet("/dummy_group/dummy_dataset").getAttribute("my_attribute_copy");
        a2_read.read(result2);

        CHECK(vec.size() == x_size);
        CHECK(result2.size() == x_size);
        CHECK(vec == result2);

        std::vector<int> v;  // with const would print a nice err msg
        file.getDataSet("/dummy_group/dummy_dataset").getAttribute("version_test").read(v);
    }

    // Delete some attributes
    {
        // From group
        auto g = file.getGroup("dummy_group");
        g.deleteAttribute("my_attribute");
        auto n = g.getNumberAttributes();
        CHECK(n == 0);

        // From dataset
        auto d = file.getDataSet("/dummy_group/dummy_dataset");
        d.deleteAttribute("my_attribute_copy");
        n = g.getNumberAttributes();
        CHECK(n == 0);
    }
}

TEST_CASE("ReadWriteAttributeVectorString") {
    readWriteAttributeVectorTest<std::string>();
}

TEMPLATE_LIST_TEST_CASE("ReadWriteAttributeVector", "[template]", dataset_test_types) {
    readWriteAttributeVectorTest<TestType>();
}

TEST_CASE("datasetOffset") {
    std::string filename = "datasetOffset.h5";
    std::string dsetname = "dset";
    const size_t size_dataset = 20;

    File file(filename, File::ReadWrite | File::Create | File::Truncate);
    std::vector<int> data(size_dataset);
    DataSet ds = file.createDataSet<int>(dsetname, DataSpace::From(data));
    ds.write(data);
    DataSet ds_read = file.getDataSet(dsetname);
    CHECK(ds_read.getOffset() > 0);
}

template <typename T>
void selectionArraySimpleTest() {
    typedef typename std::vector<T> Vector;

    std::ostringstream filename;
    filename << "h5_rw_select_test_" << typeNameHelper<T>() << "_test.h5";

    const size_t size_x = 10;
    const size_t offset_x = 2, count_x = 5;

    const std::string DATASET_NAME("dset");

    Vector values(size_x);

    ContentGenerate<T> generator;
    std::generate(values.begin(), values.end(), generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(values));

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
    const std::string FILE_NAME("h5_test_selection_multi_dim.h5");
    // Create a 2-dim dataset
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
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

    const std::string DATASET_NAME("dset");

    T values[x_size][y_size];

    ContentGenerate<T> generator;
    generate2D(values, x_size, y_size, generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims{x_size, y_size};

    DataSpace dataspace(dims);
    // Create a dataset with arbitrary type
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace);

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
    const std::string DATASET_NAME("dset");

    const size_t x_size = 10;
    const size_t y_size = 8;

    T values[x_size][y_size];

    auto file = setupHyperSlabFile(values, filename.str(), DATASET_NAME);
    auto test_cases = make_regular_hyperslab_test_data();

    for (const auto& test_case: test_cases) {
        SECTION(test_case.desc) {
            std::vector<T> result;

            file.getDataSet(DATASET_NAME).select(test_case.slab).read(result);

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

    const std::string DATASET_NAME("dset");

    const size_t x_size = 10;
    const size_t y_size = 8;

    T values[x_size][y_size];
    auto file = setupHyperSlabFile(values, filename.str(), DATASET_NAME);

    auto test_cases = make_irregular_hyperslab_test_data();

    for (const auto& test_case: test_cases) {
        SECTION(test_case.desc) {
            std::vector<T> result;

            file.getDataSet(DATASET_NAME).select(test_case.slab).read(result);

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

    const std::string DATASET_NAME("dset");

    const size_t x_size = 10;
    const size_t y_size = 8;

    T orig_values[x_size][y_size];
    auto file = setupHyperSlabFile(orig_values, filename.str(), DATASET_NAME);

    auto test_cases = make_irregular_hyperslab_test_data();

    for (const auto& test_case: test_cases) {
        SECTION(test_case.desc) {
            auto n_selected = test_case.answer.global_indices.size();
            std::vector<T> changed_values(n_selected);
            ContentGenerate<T> gen;
            std::generate(changed_values.begin(), changed_values.end(), gen);

            file.getDataSet(DATASET_NAME).select(test_case.slab).write(changed_values);

            T overwritten_values[x_size][y_size];
            file.getDataSet(DATASET_NAME).read(overwritten_values);

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

template <typename T>
void attribute_scalar_rw() {
    std::ostringstream filename;
    filename << "h5_rw_attribute_scalar_rw" << typeNameHelper<T>() << "_test.h5";

    File h5file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    ContentGenerate<T> generator;

    const T attribute_value(generator());

    Group g = h5file.createGroup("metadata");

    CHECK(!g.hasAttribute("family"));

    // write a scalar attribute
    {
        T out(attribute_value);
        Attribute att = g.createAttribute<T>("family", DataSpace::From(out));
        att.write(out);
    }

    h5file.flush();

    // test if attribute exist
    CHECK(g.hasAttribute("family"));

    // read back a scalar attribute
    {
        T res;
        Attribute att = g.getAttribute("family");
        att.read(res);
        CHECK(res == attribute_value);
    }
}

TEMPLATE_LIST_TEST_CASE("attribute_scalar_rw_all", "[template]", dataset_test_types) {
    attribute_scalar_rw<TestType>();
}

TEST_CASE("attribute_scalar_rw_string") {
    attribute_scalar_rw<std::string>();
}

// regression test https://github.com/BlueBrain/HighFive/issues/98
TEST_CASE("HighFiveOutofDimension") {
    std::string filename("h5_rw_reg_zero_dim_test.h5");

    const std::string DATASET_NAME("dset");

    {
        // Create a new file using the default property lists.
        File file(filename, File::ReadWrite | File::Create | File::Truncate);

        DataSpace d_null(DataSpace::DataspaceType::dataspace_null);

        DataSet d1 = file.createDataSet<double>(DATASET_NAME, d_null);

        file.flush();

        DataSpace recovered_d1 = d1.getSpace();

        auto ndim = recovered_d1.getNumberDimensions();
        CHECK(ndim == 0);

        auto dims = recovered_d1.getDimensions();
        CHECK(dims.size() == 0);
    }
}

template <typename T>
void readWriteShuffleDeflateTest() {
    std::ostringstream filename;
    filename << "h5_rw_deflate_" << typeNameHelper<T>() << "_test.h5";
    const std::string DATASET_NAME("dset");
    const size_t x_size = 128;
    const size_t y_size = 32;
    const size_t x_chunk = 16;
    const size_t y_chunk = 16;

    const int deflate_level = 9;

    T array[x_size][y_size];

    // write a compressed file
    {
        File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

        // Create the data space for the dataset.
        std::vector<size_t> dims{x_size, y_size};

        DataSpace dataspace(dims);

        // Use chunking
        DataSetCreateProps props;
        props.add(Chunking(std::vector<hsize_t>{x_chunk, y_chunk}));

        // Enable shuffle
        props.add(Shuffle());

        // Enable deflate
        props.add(Deflate(deflate_level));

        // Create a dataset with arbitrary type
        DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace, props);

        ContentGenerate<T> generator;
        generate2D(array, x_size, y_size, generator);

        dataset.write(array);

        file.flush();
    }

    // read it back
    {
        File file_read(filename.str(), File::ReadOnly);
        DataSet dataset_read = file_read.getDataSet("/" + DATASET_NAME);

        T result[x_size][y_size];

        dataset_read.read(result);

        for (size_t i = 0; i < x_size; ++i) {
            for (size_t j = 0; i < y_size; ++i) {
                REQUIRE(result[i][j] == array[i][j]);
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("ReadWriteShuffleDeflate", "[template]", numerical_test_types) {
    readWriteShuffleDeflateTest<TestType>();
}

template <typename T>
void readWriteSzipTest() {
    std::ostringstream filename;
    filename << "h5_rw_szip_" << typeNameHelper<T>() << "_test.h5";
    const std::string DATASET_NAME("dset");
    const size_t x_size = 128;
    const size_t y_size = 32;
    const size_t x_chunk = 8;
    const size_t y_chunk = 4;

    const int options_mask = H5_SZIP_NN_OPTION_MASK;
    const int pixels_per_block = 8;

    T array[x_size][y_size];

    // write a compressed file
    {
        File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

        // Create the data space for the dataset.
        std::vector<size_t> dims{x_size, y_size};

        DataSpace dataspace(dims);

        // Use chunking
        DataSetCreateProps props;
        props.add(Chunking(std::vector<hsize_t>{x_chunk, y_chunk}));

        // Enable szip
        props.add(Szip(options_mask, pixels_per_block));

        // Create a dataset with arbitrary type
        DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace, props);

        ContentGenerate<T> generator;
        generate2D(array, x_size, y_size, generator);

        dataset.write(array);

        file.flush();
    }

    // read it back
    {
        File file_read(filename.str(), File::ReadOnly);
        DataSet dataset_read = file_read.getDataSet("/" + DATASET_NAME);

        T result[x_size][y_size];

        dataset_read.read(result);

        for (size_t i = 0; i < x_size; ++i) {
            for (size_t j = 0; i < y_size; ++i) {
                REQUIRE(result[i][j] == array[i][j]);
            }
        }
    }
}

TEMPLATE_LIST_TEST_CASE("ReadWriteSzip", "[template]", dataset_test_types) {
    // SZIP is not consistently available across distributions.
    if (H5Zfilter_avail(H5Z_FILTER_SZIP)) {
        readWriteSzipTest<TestType>();
    } else {
        CHECK_THROWS_AS(readWriteSzipTest<TestType>(), PropertyException);
    }
}

TEST_CASE("HighFiveRecursiveGroups") {
    const std::string FILE_NAME("h5_ds_exist.h5");
    const std::string GROUP_1("group1"), GROUP_2("group2");
    const std::string DS_PATH = GROUP_1 + "/" + GROUP_2;
    const std::string DS_NAME = "ds";

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    CHECK(file.getName() == FILE_NAME);

    // Without parents creating both groups will fail
    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.createGroup(DS_PATH, false), std::exception);
    }
    Group g2 = file.createGroup(DS_PATH);

    std::vector<double> some_data{5.0, 6.0, 7.0};
    g2.createDataSet(DS_NAME, some_data);

    CHECK(file.exist(GROUP_1));

    Group g1 = file.getGroup(GROUP_1);
    CHECK(g1.exist(GROUP_2));

    // checks with full path
    CHECK(file.exist(DS_PATH));
    CHECK(file.exist(DS_PATH + "/" + DS_NAME));

    // Check with wrong middle path (before would raise Exception)
    CHECK(!file.exist(std::string("blabla/group2")));

    // Using root slash
    CHECK(file.exist(std::string("/") + DS_PATH));

    // Check unlink with existing group
    CHECK(g1.exist(GROUP_2));
    g1.unlink(GROUP_2);
    CHECK(!g1.exist(GROUP_2));

    // Check unlink with non-existing group
    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(g1.unlink("x"), HighFive::GroupException);
    }
}

TEST_CASE("HighFiveInspect") {
    const std::string FILE_NAME("group_info.h5");
    const std::string GROUP_1("group1");
    const std::string DS_NAME = "ds";

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    Group g = file.createGroup(GROUP_1);

    std::vector<double> some_data{5.0, 6.0, 7.0};
    g.createDataSet(DS_NAME, some_data);

    CHECK(file.getLinkType(GROUP_1) == LinkType::Hard);

    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.getLinkType("x"), HighFive::GroupException);
    }

    CHECK(file.getObjectType(GROUP_1) == ObjectType::Group);
    CHECK(file.getObjectType(GROUP_1 + "/" + DS_NAME) == ObjectType::Dataset);
    CHECK(g.getObjectType(DS_NAME) == ObjectType::Dataset);

    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.getObjectType(DS_NAME), HighFive::GroupException);
    }

    // Data type
    auto ds = g.getDataSet(DS_NAME);
    auto dt = ds.getDataType();
    CHECK(dt.getClass() == DataTypeClass::Float);
    CHECK(dt.getSize() == 8);
    CHECK(dt.string() == "Float64");

    // meta
    CHECK(ds.getType() == ObjectType::Dataset);  // internal
    CHECK(ds.getInfo().getRefCount() == 1);
}

TEST_CASE("HighFiveGetPath") {
    File file("getpath.h5", File::ReadWrite | File::Create | File::Truncate);

    int number = 100;
    Group group = file.createGroup("group");
    DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
    dataset.write(number);
    std::string string_list("Very important DataSet!");
    Attribute attribute = dataset.createAttribute<std::string>("attribute",
                                                               DataSpace::From(string_list));
    attribute.write(string_list);

    CHECK("/" == file.getPath());
    CHECK("/group" == group.getPath());
    CHECK("/group/data" == dataset.getPath());
    CHECK("attribute" == attribute.getName());
    CHECK("/group/data" == attribute.getPath());

    CHECK(file == dataset.getFile());
    CHECK(file == attribute.getFile());

    // Destroy file early (it should live inside Dataset/Group)
    std::unique_ptr<File> f2(new File("getpath.h5"));
    const auto& d2 = f2->getDataSet("/group/data");
    f2.reset(nullptr);
    CHECK(d2.getFile().getPath() == "/");
}

TEST_CASE("HighFiveSoftLinks") {
    const std::string FILE_NAME("softlinks.h5");
    const std::string DS_PATH("/hard_link/dataset");
    const std::string LINK_PATH("/soft_link/to_ds");
    const std::vector<int> data{11, 22, 33};

    {
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
        auto dset = file.createDataSet(DS_PATH, data);
        file.createSoftLink(LINK_PATH, dset);
    }

    {
        File file(FILE_NAME, File::ReadWrite);
        std::vector<int> data_out;
        file.getDataSet(LINK_PATH).read(data_out);
        CHECK(data == data_out);
    }

    {
        const std::string EXTERNAL_LINK_PATH("/external_link/to_ds");
        File file2("link_external_to.h5", File::ReadWrite | File::Create | File::Truncate);
        file2.createExternalLink(EXTERNAL_LINK_PATH, FILE_NAME, DS_PATH);

        std::vector<int> data_out;
        file2.getDataSet(EXTERNAL_LINK_PATH).read(data_out);
        CHECK(data == data_out);
    }
}

TEST_CASE("HighFiveRename") {
    File file("move.h5", File::ReadWrite | File::Create | File::Truncate);

    int number = 100;

    {
        Group group = file.createGroup("group");
        DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
        dataset.write(number);
        std::string path = dataset.getPath();
        CHECK("/group/data" == path);
    }

    file.rename("/group/data", "/new/group/new/data");

    {
        DataSet dataset = file.getDataSet("/new/group/new/data");
        std::string path = dataset.getPath();
        CHECK("/new/group/new/data" == path);
        int read;
        dataset.read(read);
        CHECK(number == read);
    }
}

TEST_CASE("HighFiveRenameRelative") {
    File file("move.h5", File::ReadWrite | File::Create | File::Truncate);
    Group group = file.createGroup("group");

    int number = 100;

    {
        DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
        dataset.write(number);
        CHECK("/group/data" == dataset.getPath());
    }

    group.rename("data", "new_data");

    {
        DataSet dataset = group.getDataSet("new_data");
        CHECK("/group/new_data" == dataset.getPath());
        int read;
        dataset.read(read);
        CHECK(number == read);
    }
}

TEST_CASE("HighFivePropertyObjects") {
    const auto& plist1 = FileCreateProps::Default();  // get const-ref, otherwise copies
    CHECK(plist1.getId() == H5P_DEFAULT);
    CHECK(!plist1.isValid());  // not valid -> no inc_ref
    auto plist2 = plist1;      // copy  (from Object)
    CHECK(plist2.getId() == H5P_DEFAULT);

    // Underlying object is same (singleton holder of H5P_DEFAULT)
    const auto& other_plist_type = LinkCreateProps::Default();
    CHECK((void*) &plist1 == (void*) &other_plist_type);

    LinkCreateProps plist_g;
    CHECK(plist_g.getId() == H5P_DEFAULT);
    CHECK(!plist_g.isValid());

    plist_g.add(CreateIntermediateGroup());
    CHECK(plist_g.isValid());
    auto plist_g2 = plist_g;
    CHECK(plist_g2.isValid());
}

struct CSL1 {
    int m1;
    int m2;
    int m3;
};

struct CSL2 {
    CSL1 csl1;
};

CompoundType create_compound_csl1() {
    auto t2 = AtomicType<int>();
    CompoundType t1({{"m1", AtomicType<int>{}}, {"m2", AtomicType<int>{}}, {"m3", t2}});

    return t1;
}

CompoundType create_compound_csl2() {
    CompoundType t1 = create_compound_csl1();

    CompoundType t2({{"csl1", t1}});

    return t2;
}

HIGHFIVE_REGISTER_TYPE(CSL1, create_compound_csl1)
HIGHFIVE_REGISTER_TYPE(CSL2, create_compound_csl2)

TEST_CASE("HighFiveCompounds") {
    const std::string FILE_NAME("compounds_test.h5");
    const std::string DATASET_NAME1("/a");
    const std::string DATASET_NAME2("/b");

    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    auto t3 = AtomicType<int>();
    CompoundType t1 = create_compound_csl1();
    t1.commit(file, "my_type");

    CompoundType t2 = create_compound_csl2();
    t2.commit(file, "my_type2");

    {  // Not nested
        auto dataset = file.createDataSet(DATASET_NAME1, DataSpace(2), t1);

        std::vector<CSL1> csl = {{1, 1, 1}, {2, 3, 4}};
        dataset.write(csl);

        file.flush();

        std::vector<CSL1> result;
        dataset.select({0}, {2}).read(result);

        CHECK(result.size() == 2);
        CHECK(result[0].m1 == 1);
        CHECK(result[0].m2 == 1);
        CHECK(result[0].m3 == 1);
        CHECK(result[1].m1 == 2);
        CHECK(result[1].m2 == 3);
        CHECK(result[1].m3 == 4);
    }

    {  // Nested
        auto dataset = file.createDataSet(DATASET_NAME2, DataSpace(2), t2);

        std::vector<CSL2> csl = {{{1, 1, 1}, {2, 3, 4}}};
        dataset.write(csl);

        file.flush();
        std::vector<CSL2> result = {{{1, 1, 1}, {2, 3, 4}}};
        dataset.select({0}, {2}).read(result);

        CHECK(result.size() == 2);
        CHECK(result[0].csl1.m1 == 1);
        CHECK(result[0].csl1.m2 == 1);
        CHECK(result[0].csl1.m3 == 1);
        CHECK(result[1].csl1.m1 == 2);
        CHECK(result[1].csl1.m2 == 3);
        CHECK(result[1].csl1.m3 == 4);
    }

    // Test the constructor from hid
    CompoundType t1_from_hid(t1);
    CHECK(t1 == t1_from_hid);

    CompoundType t2_from_hid(t2);
    CHECK(t2 == t2_from_hid);
}

struct GrandChild {
    uint32_t gcm1;
    uint32_t gcm2;
    uint32_t gcm3;
};

struct Child {
    GrandChild grandChild;
    uint32_t cm1;
};

struct Parent {
    uint32_t pm1;
    Child child;
};

CompoundType create_compound_GrandChild() {
    auto t2 = AtomicType<uint32_t>();
    CompoundType t1({{"gcm1", AtomicType<uint32_t>{}},
                     {"gcm2", AtomicType<uint32_t>{}},
                     {
                         "gcm3",
                         t2,
                     }});
    return t1;
}

CompoundType create_compound_Child() {
    auto nestedType = create_compound_GrandChild();
    return CompoundType{{{
                             "grandChild",
                             nestedType,
                         },
                         {"cm1", AtomicType<uint32_t>{}}}};
}

CompoundType create_compound_Parent() {
    auto nestedType = create_compound_Child();
    return CompoundType{{{"pm1", AtomicType<uint32_t>{}},
                         {
                             "child",
                             nestedType,
                         }}};
}

HIGHFIVE_REGISTER_TYPE(GrandChild, create_compound_GrandChild)
HIGHFIVE_REGISTER_TYPE(Child, create_compound_Child)
HIGHFIVE_REGISTER_TYPE(Parent, create_compound_Parent)

TEST_CASE("HighFiveCompoundsNested") {
    const std::string FILE_NAME("nested_compounds_test.h5");
    const std::string DATASET_NAME("/a");

    {  // Write
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
        auto type = create_compound_Parent();

        auto dataset = file.createDataSet(DATASET_NAME, DataSpace(2), type);
        CHECK(dataset.getDataType().getSize() == 20);

        std::vector<Parent> csl = {Parent{1, Child{GrandChild{1, 1, 1}, 1}},
                                   Parent{2, Child{GrandChild{3, 4, 5}, 6}}};
        dataset.write(csl);
    }

    {  // Read
        File file(FILE_NAME, File::ReadOnly);
        std::vector<Parent> result;
        auto dataset = file.getDataSet(DATASET_NAME);
        CHECK(dataset.getDataType().getSize() == 20);
        dataset.select({0}, {2}).read(result);

        CHECK(result.size() == 2);
        CHECK(result[0].pm1 == 1);
        CHECK(result[0].child.grandChild.gcm1 == 1);
        CHECK(result[0].child.grandChild.gcm2 == 1);
        CHECK(result[0].child.grandChild.gcm3 == 1);
        CHECK(result[0].child.cm1 == 1);
        CHECK(result[1].pm1 == 2);
        CHECK(result[1].child.grandChild.gcm1 == 3);
        CHECK(result[1].child.grandChild.gcm2 == 4);
        CHECK(result[1].child.grandChild.gcm3 == 5);
        CHECK(result[1].child.cm1 == 6);
    }
}

template <size_t N>
struct Record {
    double d = 3.14;
    int i = 42;
    char s[N];
};

template <size_t N>
void fill(Record<N>& r) {
    constexpr char ref[] = "123456789a123456789b123456789c123456789d123456789e123456789f";
    std::copy(ref, ref + N - 1, r.s);
    r.s[N - 1] = '\0';
}

template <size_t N>
CompoundType rec_t() {
    using RecN = Record<N>;
    return {{"d", create_datatype<decltype(RecN::d)>()},
            {"i", create_datatype<decltype(RecN::i)>()},
            {"s", create_datatype<decltype(RecN::s)>()}};
}

HIGHFIVE_REGISTER_TYPE(Record<4>, rec_t<4>)
HIGHFIVE_REGISTER_TYPE(Record<8>, rec_t<8>)
HIGHFIVE_REGISTER_TYPE(Record<9>, rec_t<9>)

template <size_t N>
void save(File& f) {
    const size_t numRec = 2;
    std::vector<Record<N>> recs(numRec);
    fill<N>(recs[0]);
    fill<N>(recs[1]);
    auto dataset = f.createDataSet<Record<N>>("records" + std::to_string(N), DataSpace::From(recs));
    dataset.write(recs);
}

template <size_t N>
std::string check(File& f) {
    const size_t numRec = 2;
    std::vector<Record<N>> recs(numRec);
    f.getDataSet("records" + std::to_string(N)).read(recs);
    return std::string(recs[0].s);
}

TEST_CASE("HighFiveCompoundsSeveralPadding") {
    const std::string FILE_NAME("padded_compounds_test.h5");

    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    {  // Write
        // 4 have been choose because no padding
        // /* offset      |    size */  type = struct Record<4> {
        // /*      0      |       8 */    double d;
        // /*      8      |       4 */    int i;
        // /*     12      |       4 */    char s[4];
        // total size (bytes):   16
        CHECK_NOTHROW(save<4>(file));
        // 8 have been choose because there is a padding
        // /* offset      |    size */  type = struct Record<8> {
        // /*      0      |       8 */    double d;
        // /*      8      |       4 */    int i;
        // /*     12      |       8 */    char s[8];
        // /* XXX  4-byte padding   */
        // total size (bytes):   24
        CHECK_NOTHROW(save<8>(file));
        // 9 have been choose because there should not be a padding on 9
        // /* offset      |    size */  type = struct Record<9> {
        // /*      0      |       8 */    double d;
        // /*      8      |       4 */    int i;
        // /*     12      |       9 */    char s[9];
        // /* XXX  3-byte padding   */
        // total size (bytes):   24
        CHECK_NOTHROW(save<9>(file));
    }

    {  // Read
        CHECK(check<4>(file) == std::string("123"));
        CHECK(check<8>(file) == std::string("1234567"));
        CHECK(check<9>(file) == std::string("12345678"));
    }
}

enum Position {
    FIRST = 1,
    SECOND = 2,
    THIRD = 3,
    LAST = -1,
};

enum class Direction : signed char {
    FORWARD = 1,
    BACKWARD = -1,
    LEFT = -2,
    RIGHT = 2,
};

// This is only for boost test
std::ostream& operator<<(std::ostream& ost, const Direction& dir) {
    ost << static_cast<int>(dir);
    return ost;
}

EnumType<Position> create_enum_position() {
    return {{"FIRST", Position::FIRST},
            {"SECOND", Position::SECOND},
            {"THIRD", Position::THIRD},
            {"LAST", Position::LAST}};
}
HIGHFIVE_REGISTER_TYPE(Position, create_enum_position)

EnumType<Direction> create_enum_direction() {
    return {{"FORWARD", Direction::FORWARD},
            {"BACKWARD", Direction::BACKWARD},
            {"LEFT", Direction::LEFT},
            {"RIGHT", Direction::RIGHT}};
}
HIGHFIVE_REGISTER_TYPE(Direction, create_enum_direction)

TEST_CASE("HighFiveEnum") {
    const std::string FILE_NAME("enum_test.h5");
    const std::string DATASET_NAME1("/a");
    const std::string DATASET_NAME2("/b");

    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    {  // Unscoped enum
        auto e1 = create_enum_position();
        e1.commit(file, "Position");

        auto dataset = file.createDataSet(DATASET_NAME1, DataSpace(1), e1);
        dataset.write(Position::FIRST);

        file.flush();

        Position result;
        dataset.select(ElementSet({0})).read(result);

        CHECK(result == Position::FIRST);
    }

    {  // Scoped enum
        auto e1 = create_enum_direction();
        e1.commit(file, "Direction");

        auto dataset = file.createDataSet(DATASET_NAME2, DataSpace(5), e1);
        std::vector<Direction> robot_moves({Direction::BACKWARD,
                                            Direction::FORWARD,
                                            Direction::FORWARD,
                                            Direction::LEFT,
                                            Direction::LEFT});
        dataset.write(robot_moves);

        file.flush();

        std::vector<Direction> result;
        dataset.read(result);

        CHECK(result[0] == Direction::BACKWARD);
        CHECK(result[1] == Direction::FORWARD);
        CHECK(result[2] == Direction::FORWARD);
        CHECK(result[3] == Direction::LEFT);
        CHECK(result[4] == Direction::LEFT);
    }
}

TEST_CASE("HighFiveFixedString") {
    const std::string FILE_NAME("array_atomic_types.h5");
    const std::string GROUP_1("group1");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    char raw_strings[][10] = {"abcd", "1234"};

    /// This will not compile - only char arrays - hits static_assert with a nice
    /// error
    // file.createDataSet<int[10]>(DS_NAME, DataSpace(2)));

    {  // But char should be fine
        auto ds = file.createDataSet<char[10]>("ds1", DataSpace(2));
        CHECK(ds.getDataType().getClass() == DataTypeClass::String);
        ds.write(raw_strings);
    }

    {  // char[] is, by default, int8
        auto ds2 = file.createDataSet("ds2", raw_strings);
        CHECK(ds2.getDataType().getClass() == DataTypeClass::Integer);
    }

    {  // String Truncate happens low-level if well setup
        auto ds3 = file.createDataSet<char[6]>("ds3", DataSpace::FromCharArrayStrings(raw_strings));
        ds3.write(raw_strings);
    }

    {  // Write as raw elements from pointer (with const)
        const char(*strings_fixed)[10] = raw_strings;
        // With a pointer we dont know how many strings -> manual DataSpace
        file.createDataSet<char[10]>("ds4", DataSpace(2)).write(strings_fixed);
    }

    {  // Cant convert flex-length to fixed-length
        const char* buffer[] = {"abcd", "1234"};
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.createDataSet<char[10]>("ds5", DataSpace(2)).write(buffer),
                        HighFive::DataSetException);
    }

    {  // scalar char strings
        const char buffer[] = "abcd";
        file.createDataSet<char[10]>("ds6", DataSpace(1)).write(buffer);
    }

    {  // Dedicated FixedLenStringArray
        FixedLenStringArray<10> arr{"0000000", "1111111"};
        // For completeness, test also the other constructor
        FixedLenStringArray<10> arrx(std::vector<std::string>{"0000", "1111"});

        // More API: test inserting something
        arr.push_back("2222");
        auto ds = file.createDataSet("ds7", arr);  // Short syntax ok

        // Recover truncating
        FixedLenStringArray<4> array_back;
        ds.read(array_back);
        CHECK(array_back.size() == 3);
        CHECK(array_back[0] == std::string("000"));
        CHECK(array_back[1] == std::string("111"));
        CHECK(array_back[2] == std::string("222"));
        CHECK(array_back.getString(1) == "111");
        CHECK(array_back.front() == std::string("000"));
        CHECK(array_back.back() == std::string("222"));
        CHECK(array_back.data() == std::string("000"));
        array_back.data()[0] = 'x';
        CHECK(array_back.data() == std::string("x00"));

        for (auto& raw_elem: array_back) {
            raw_elem[1] = 'y';
        }
        CHECK(array_back.getString(1) == "1y1");
        for (auto iter = array_back.cbegin(); iter != array_back.cend(); ++iter) {
            CHECK((*iter)[1] == 'y');
        }
    }
}

TEST_CASE("HighFiveFixedLenStringArrayStructure") {
    using fixed_array_t = FixedLenStringArray<10>;
    // increment the characters of a string written in a std::array
    auto increment_string = [](const fixed_array_t::value_type arr) {
        fixed_array_t::value_type output(arr);
        for (auto& c: output) {
            if (c == 0) {
                break;
            }
            ++c;
        }
        return output;
    };

    // manipulate FixedLenStringArray with std::copy
    {
        const fixed_array_t arr1{"0000000", "1111111"};
        fixed_array_t arr2{"0000000", "1111111"};
        std::copy(arr1.begin(), arr1.end(), std::back_inserter(arr2));
        CHECK(arr2.size() == 4);
    }

    // manipulate FixedLenStringArray with std::transform
    {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::transform(arr1.begin(), arr1.end(), std::back_inserter(arr), increment_string);
        }
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("1111111"));
        CHECK(arr[1] == std::string("2222222"));
    }

    // manipulate FixedLenStringArray with std::transform and reverse iterator
    {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::copy(arr1.rbegin(), arr1.rend(), std::back_inserter(arr));
        }
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("1111111"));
        CHECK(arr[1] == std::string("0000000"));
    }

    // manipulate FixedLenStringArray with std::remove_copy_if
    {
        fixed_array_t arr2;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::remove_copy_if(arr1.begin(),
                                arr1.end(),
                                std::back_inserter(arr2),
                                [](const fixed_array_t::value_type& s) {
                                    return std::strncmp(s.data(), "1111111", 7) == 0;
                                });
        }
        CHECK(arr2.size() == 1);
        CHECK(arr2[0] == std::string("0000000"));
    }
}

TEST_CASE("HighFiveFixedLenStringArrayAttribute") {
    const std::string FILE_NAME("fixed_array_attr.h5");
    // Create a new file using the default property lists.
    {
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
        FixedLenStringArray<10> arr{"Hello", "world"};
        file.createAttribute("str", arr);
    }
    // Re-read it
    {
        File file(FILE_NAME);
        FixedLenStringArray<8> arr;  // notice the output strings can be smaller
        file.getAttribute("str").read(arr);
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("Hello"));
        CHECK(arr[1] == std::string("world"));
    }
}

TEST_CASE("HighFiveReference") {
    const std::string FILE_NAME("h5_ref_test.h5");
    const std::string DATASET1_NAME("dset1");
    const std::string DATASET2_NAME("dset2");
    const std::string GROUP_NAME("/group1");
    const std::string REFGROUP_NAME("/group2");
    const std::string REFDATASET_NAME("dset2");

    ContentGenerate<double> generator;
    std::vector<double> vec1(4);
    std::vector<double> vec2(4);
    std::generate(vec1.begin(), vec1.end(), generator);
    std::generate(vec2.begin(), vec2.end(), generator);
    {
        // Create a new file using the default property lists.
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // create group
        Group g1 = file.createGroup(GROUP_NAME);

        // create datasets and write some data
        DataSet dataset1 = g1.createDataSet(DATASET1_NAME, vec1);
        DataSet dataset2 = g1.createDataSet(DATASET2_NAME, vec2);

        // create group to hold reference
        Group refgroup = file.createGroup(REFGROUP_NAME);

        // create the references and write them into a new dataset inside refgroup
        auto references = std::vector<Reference>({{g1, dataset1}, {file, g1}});
        DataSet ref_ds = refgroup.createDataSet(REFDATASET_NAME, references);
    }
    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);
        Group refgroup = file.getGroup(REFGROUP_NAME);

        DataSet refdataset = refgroup.getDataSet(REFDATASET_NAME);
        CHECK(2 == refdataset.getSpace().getDimensions()[0]);
        auto refs = std::vector<Reference>();
        refdataset.read(refs);
        CHECK_THROWS_AS(refs[0].dereference<Group>(file), HighFive::ReferenceException);
        auto data_ds = refs[0].dereference<DataSet>(file);
        std::vector<double> rdata;
        data_ds.read(rdata);
        for (size_t i = 0; i < rdata.size(); ++i) {
            CHECK(rdata[i] == vec1[i]);
        }

        auto group = refs[1].dereference<Group>(file);
        DataSet data_ds2 = group.getDataSet(DATASET2_NAME);
        std::vector<double> rdata2;
        data_ds2.read(rdata2);
        for (size_t i = 0; i < rdata2.size(); ++i) {
            CHECK(rdata2[i] == vec2[i]);
        }
    }
}

TEST_CASE("HighFiveReadWriteConsts") {
    const std::string FILE_NAME("3d_dataset_from_flat.h5");
    const std::string DATASET_NAME("dset");
    const std::array<std::size_t, 3> DIMS{3, 3, 3};
    using datatype = int;

    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    DataSpace dataspace = DataSpace(DIMS);

    DataSet dataset = file.createDataSet<datatype>(DATASET_NAME, dataspace);
    std::vector<datatype> const t1(DIMS[0] * DIMS[1] * DIMS[2], 1);
    auto raw_3d_vec_const = reinterpret_cast<datatype const* const* const*>(t1.data());
    dataset.write(raw_3d_vec_const);

    std::vector<std::vector<std::vector<datatype>>> result;
    dataset.read(result);
    for (const auto& vec2d: result) {
        for (const auto& vec1d: vec2d) {
            REQUIRE(vec1d == (std::vector<datatype>{1, 1, 1}));
        }
    }
}

TEST_CASE("HighFiveDataTypeClass") {
    auto Float = DataTypeClass::Float;
    auto String = DataTypeClass::String;
    auto Invalid = DataTypeClass::Invalid;

    CHECK(Float != Invalid);

    CHECK((Float & Float) == Float);
    CHECK((Float | Float) == Float);

    CHECK((Float & String) == Invalid);
    CHECK((Float | String) != Invalid);

    CHECK(((Float & String) & Float) == Invalid);
    CHECK(((Float | String) & Float) == Float);
    CHECK(((Float | String) & String) == String);
}

#ifdef H5_USE_EIGEN

template <typename T>
void test_eigen_vec(File& file, const std::string& test_flavor, const T& vec_input, T& vec_output) {
    const std::string DS_NAME = "ds";
    file.createDataSet(DS_NAME + test_flavor, vec_input).write(vec_input);
    file.getDataSet(DS_NAME + test_flavor).read(vec_output);
    CHECK(vec_input == vec_output);
}

TEST_CASE("HighFiveEigen") {
    const std::string FILE_NAME("test_eigen.h5");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    std::string DS_NAME_FLAVOR;

    // std::vector<of vector <of POD>>
    {
        DS_NAME_FLAVOR = "VectorOfVectorOfPOD";
        std::vector<std::vector<float>> vec_in{{5.0f, 6.0f, 7.0f},
                                               {5.1f, 6.1f, 7.1f},
                                               {5.2f, 6.2f, 7.2f}};
        std::vector<std::vector<float>> vec_out;
        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

    // std::vector<Eigen::Vector3d>
    {
        DS_NAME_FLAVOR = "VectorOfEigenVector3d";
        std::vector<Eigen::Vector3d> vec_in{{5.0, 6.0, 7.0}, {7.0, 8.0, 9.0}};
        std::vector<Eigen::Vector3d> vec_out;
        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

    // Eigen Vector2d
    {
        DS_NAME_FLAVOR = "EigenVector2d";
        Eigen::Vector2d vec_in{5.0, 6.0};
        Eigen::Vector2d vec_out;

        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

    // Eigen Matrix
    {
        DS_NAME_FLAVOR = "EigenMatrix";
        Eigen::Matrix<double, 3, 3> vec_in;
        vec_in << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        Eigen::Matrix<double, 3, 3> vec_out;

        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

    // Eigen MatrixXd
    {
        DS_NAME_FLAVOR = "EigenMatrixXd";
        Eigen::MatrixXd vec_in = 100. * Eigen::MatrixXd::Random(20, 5);
        Eigen::MatrixXd vec_out(20, 5);

        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

    // std::vector<of EigenMatrixXd>
    {
        DS_NAME_FLAVOR = "VectorEigenMatrixXd";

        Eigen::MatrixXd m1 = 100. * Eigen::MatrixXd::Random(20, 5);
        Eigen::MatrixXd m2 = 100. * Eigen::MatrixXd::Random(20, 5);
        std::vector<Eigen::MatrixXd> vec_in;
        vec_in.push_back(m1);
        vec_in.push_back(m2);
        std::vector<Eigen::MatrixXd> vec_out(2, Eigen::MatrixXd::Zero(20, 5));

        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

#ifdef H5_USE_BOOST
    // boost::multi_array<of EigenVector3f>
    {
        DS_NAME_FLAVOR = "BMultiEigenVector3f";

        boost::multi_array<Eigen::Vector3f, 3> vec_in(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_in[i][j][k] = Eigen::Vector3f::Random(3);
                }
            }
        }
        boost::multi_array<Eigen::Vector3f, 3> vec_out(boost::extents[3][2][2]);

        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

    // boost::multi_array<of EigenMatrixXd>
    {
        DS_NAME_FLAVOR = "BMultiEigenMatrixXd";

        boost::multi_array<Eigen::MatrixXd, 3> vec_in(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_in[i][j][k] = Eigen::MatrixXd::Random(3, 3);
                }
            }
        }
        boost::multi_array<Eigen::MatrixXd, 3> vec_out(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_out[i][j][k] = Eigen::MatrixXd::Zero(3, 3);
                }
            }
        test_eigen_vec(file, DS_NAME_FLAVOR, vec_in, vec_out);
    }

#endif
}
#endif
