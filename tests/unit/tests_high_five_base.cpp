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
#include <cstring>
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

using namespace HighFive;
using Catch::Matchers::Equals;

TEST_CASE("Basic HighFive tests") {
    const std::string file_name("h5tutr_dset.h5");
    const std::string dataset_name("dset");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    CHECK(file.getName() == file_name);

    // Create the data space for the dataset.
    std::vector<size_t> dims{4, 6};

    DataSpace dataspace(dims);

    // check if the dataset exist
    CHECK(!file.exist(dataset_name + "_double"));

    // Create a dataset with double precision floating points
    DataSet dataset_double =
        file.createDataSet(dataset_name + "_double", dataspace, AtomicType<double>());

    CHECK(file.getObjectName(0) == dataset_name + "_double");

    {
        // check if it exist again
        CHECK(file.exist(dataset_name + "_double"));

        // and also try to recreate it to the sake of exception testing
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.createDataSet(dataset_name + "_double",
                                           dataspace,
                                           AtomicType<double>()),
                        DataSetException);
    }

    DataSet dataset_size_t = file.createDataSet<size_t>(dataset_name + "_size_t", dataspace);
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
    const std::string file_name("openmodes.h5");

    std::remove(file_name.c_str());

    SilenceHDF5 silencer;

    // Attempt open file only ReadWrite should fail (wont create)
    CHECK_THROWS_AS(File(file_name, File::ReadWrite), FileException);

    // But with Create flag should be fine
    { File file(file_name, File::ReadWrite | File::Create); }

    // But if its there and exclusive is given, should fail
    CHECK_THROWS_AS(File(file_name, File::ReadWrite | File::Excl), FileException);
    // ReadWrite and Excl flags are fine together (posix)
    std::remove(file_name.c_str());
    { File file(file_name, File::ReadWrite | File::Excl); }
    // All three are fine as well (as long as the file does not exist)
    std::remove(file_name.c_str());
    { File file(file_name, File::ReadWrite | File::Create | File::Excl); }

    // Just a few combinations are incompatible, detected by hdf5lib
    CHECK_THROWS_AS(File(file_name, File::Truncate | File::Excl), FileException);

    std::remove(file_name.c_str());
    CHECK_THROWS_AS(File(file_name, File::Truncate | File::Excl), FileException);

    // But in most cases we will truncate and that should always work
    { File file(file_name, File::Truncate); }
    std::remove(file_name.c_str());
    { File file(file_name, File::Truncate); }

    // Last but not least, defaults should be ok
    { File file(file_name); }     // ReadOnly
    { File file(file_name, 0); }  // force empty-flags, does open without flags
}

TEST_CASE("Test file version bounds") {
    const std::string file_name("h5_version_bounds.h5");

    std::remove(file_name.c_str());

    {
        File file(file_name, File::Truncate);
        auto bounds = file.getVersionBounds();
        CHECK(bounds.first == H5F_LIBVER_EARLIEST);
        CHECK(bounds.second == H5F_LIBVER_LATEST);
    }

    std::remove(file_name.c_str());

    {
        FileAccessProps fapl;
        fapl.add(FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
        File file(file_name, File::Truncate, fapl);
        auto bounds = file.getVersionBounds();
        CHECK(bounds.first == H5F_LIBVER_LATEST);
        CHECK(bounds.second == H5F_LIBVER_LATEST);
    }
}

#if H5_VERSION_GE(1, 10, 1)
TEST_CASE("Test file space strategy") {
    const std::string file_name("h5_file_space_strategy.h5");
    auto strategies = std::vector<H5F_fspace_strategy_t>{H5F_FSPACE_STRATEGY_FSM_AGGR,
                                                         H5F_FSPACE_STRATEGY_AGGR,
                                                         H5F_FSPACE_STRATEGY_PAGE,
                                                         H5F_FSPACE_STRATEGY_NONE};

    for (const auto& strategy: strategies) {
        {
            FileCreateProps create_props;
            create_props.add(FileSpaceStrategy(strategy, true, 0));

            File file(file_name, File::Truncate, create_props);
        }

        {
            File file(file_name, File::ReadOnly);
            CHECK(file.getFileSpaceStrategy() == strategy);
        }
    }
}

TEST_CASE("Test file space page size") {
    const std::string file_name("h5_file_space_page_size.h5");
    hsize_t page_size = 1024;
    {
        FileCreateProps create_props;
        create_props.add(FileSpaceStrategy(H5F_FSPACE_STRATEGY_PAGE, true, 0));
        create_props.add(FileSpacePageSize(page_size));

        File file(file_name, File::Truncate, create_props);
    }

    {
        File file(file_name, File::ReadOnly);
        CHECK(file.getFileSpacePageSize() == page_size);
    }
}

#ifndef H5_HAVE_PARALLEL
TEST_CASE("Test page buffer size") {
    const std::string file_name("h5_page_buffer_size.h5");
    hsize_t page_size = 1024;
    {
        FileCreateProps create_props;
        create_props.add(FileSpaceStrategy(H5F_FSPACE_STRATEGY_PAGE, true, 0));
        create_props.add(FileSpacePageSize(page_size));

        FileAccessProps access_props;
        access_props.add(FileVersionBounds(H5F_LIBVER_V110, H5F_LIBVER_V110));

        File file(file_name, File::Truncate, create_props, access_props);

        file.createDataSet("x", std::vector<double>{1.0, 2.0, 3.0});
    }

    {
        FileAccessProps access_props;
        access_props.add(PageBufferSize(1024));

        File file(file_name, File::ReadOnly, access_props);

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
    const std::string file_name("h5_meta_block_size.h5");

    std::remove(file_name.c_str());

    {
        File file(file_name, File::Truncate);
        // Default for HDF5
        CHECK(file.getMetadataBlockSize() == 2048);
    }

    std::remove(file_name.c_str());

    {
        FileAccessProps fapl;
        fapl.add(MetadataBlockSize(10240));
        File file(file_name, File::Truncate, fapl);
        CHECK(file.getMetadataBlockSize() == 10240);
    }
}

TEST_CASE("Test group properties") {
    const std::string file_name("h5_group_properties.h5");
    FileAccessProps fapl;
    // When using hdf5 1.10.2 and later, the lower bound may be set to
    // H5F_LIBVER_V18
    fapl.add(FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    File file(file_name, File::Truncate, fapl);

    GroupCreateProps props;
    props.add(EstimatedLinkInfo(10, 60));
    auto group = file.createGroup("g", props);
    auto sizes = group.getEstimatedLinkInfo();

    CHECK(sizes.first == 10);
    CHECK(sizes.second == 60);
}

TEST_CASE("Test allocation time") {
    const std::string file_name("h5_dataset_alloc_time.h5");
    File file(file_name, File::Truncate);

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

/*
 * Test to ensure legacy support: DataSet used to have a default constructor.
 * However, it is not useful to have a DataSet object that does not actually
 * refer to a dataset in a file. Hence, the the default constructor was
 * deprecated.
 * This test is to ensure that the constructor is not accidentally removed and
 * thereby break users' code.
 */
TEST_CASE("Test default constructors") {
    const std::string file_name("h5_default_ctors.h5");
    const std::string dataset_name("dset");
    File file(file_name, File::Truncate);
    auto ds = file.createDataSet(dataset_name, std::vector<int>{1, 2, 3, 4, 5});

    DataSet d2;  // expect deprecation warning, as it constructs unsafe object
    // d2.getFile();  // runtime error
    CHECK(!d2.isValid());
    d2 = ds;  // copy
    CHECK(d2.isValid());
}

TEST_CASE("Test groups and datasets") {
    const std::string file_name("h5_group_test.h5");
    const std::string dataset_name("dset");
    const std::string chunked_dataset_name("chunked_dset");
    const std::string chunked_dataset_small_name("chunked_dset_small");
    const std::string group_name_1("/group1");
    const std::string group_name_2("group2");
    const std::string group_nested_name("group_nested");

    {
        // Create a new file using the default property lists.
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);

        // absolute group
        file.createGroup(group_name_1);
        // nested group absolute
        file.createGroup(group_name_1 + "/" + group_nested_name);
        // relative group
        Group g1 = file.createGroup(group_name_2);
        // relative group
        Group nested = g1.createGroup(group_nested_name);

        // Create the data space for the dataset.
        std::vector<size_t> dims{4, 6};

        DataSpace dataspace(dims);

        DataSet dataset_absolute = file.createDataSet(group_name_1 + "/" + group_nested_name + "/" +
                                                          dataset_name,
                                                      dataspace,
                                                      AtomicType<double>());

        DataSet dataset_relative =
            nested.createDataSet(dataset_name, dataspace, AtomicType<double>());

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
            CHECK_THROWS_AS(file.createDataSet(chunked_dataset_name,
                                               dataspace,
                                               AtomicType<double>(),
                                               badChunking0),
                            DataSetException);

            CHECK_THROWS_AS(file.createDataSet(chunked_dataset_name,
                                               dataspace,
                                               AtomicType<double>(),
                                               badChunking1),
                            DataSetException);
        }

        // here we use the other signature
        DataSet dataset_chunked =
            file.createDataSet<float>(chunked_dataset_name, dataspace, goodChunking, cacheConfig);

        // Here we resize to smaller than the chunking size
        DataSet dataset_chunked_small =
            file.createDataSet<float>(chunked_dataset_small_name, dataspace, goodChunking);

        dataset_chunked_small.resize({1, 1});
    }
    // read it back
    {
        File file(file_name, File::ReadOnly);
        Group g1 = file.getGroup(group_name_1);
        Group g2 = file.getGroup(group_name_2);
        Group nested_group2 = g2.getGroup(group_nested_name);

        DataSet dataset_absolute = file.getDataSet(group_name_1 + "/" + group_nested_name + "/" +
                                                   dataset_name);
        CHECK(4 == dataset_absolute.getSpace().getDimensions()[0]);

        DataSet dataset_relative = nested_group2.getDataSet(dataset_name);
        CHECK(4 == dataset_relative.getSpace().getDimensions()[0]);

        DataSetAccessProps accessProps;
        accessProps.add(Caching(13, 1024, 0.5));
        DataSet dataset_chunked = file.getDataSet(chunked_dataset_name, accessProps);
        CHECK(4 == dataset_chunked.getSpace().getDimensions()[0]);

        DataSet dataset_chunked_small = file.getDataSet(chunked_dataset_small_name);
        CHECK(1 == dataset_chunked_small.getSpace().getDimensions()[0]);
    }
}

TEST_CASE("FileSpace") {
    const std::string filename = "filespace.h5";
    const std::string ds_path = "dataset";
    const std::vector<int> data{13, 24, 36};

    File file(filename, File::Truncate);
    file.createDataSet(ds_path, data);

    CHECK(file.getFileSize() > 0);
}

TEST_CASE("FreeSpace (default)") {
    const std::string filename = "freespace_default.h5";
    const std::string ds_path = "dataset";
    const std::vector<int> data{13, 24, 36};

    {
        File file(filename, File::Truncate);
        auto dset = file.createDataSet(ds_path, data);
    }

    {
        File file(filename, File::ReadWrite);
        file.unlink(ds_path);
        CHECK(file.getFreeSpace() > 0);
        CHECK(file.getFreeSpace() < file.getFileSize());
    }
}

#if H5_VERSION_GE(1, 10, 1)
TEST_CASE("FreeSpace (tracked)") {
    const std::string filename = "freespace_tracked.h5";
    const std::string ds_path = "dataset";
    const std::vector<int> data{13, 24, 36};

    {
        FileCreateProps fcp;
        fcp.add(FileSpaceStrategy(H5F_FSPACE_STRATEGY_FSM_AGGR, true, 0));
        File file(filename, File::Truncate, fcp);
        auto dset = file.createDataSet(ds_path, data);
    }

    {
        File file(filename, File::ReadWrite);
        file.unlink(ds_path);

#if H5_VERSION_GE(1, 12, 0)
        // This fails on 1.10.x but starts working in 1.12.0
        CHECK(file.getFreeSpace() > 0);
#endif
        CHECK(file.getFreeSpace() < file.getFileSize());
    }

    {
        File file(filename, File::ReadOnly);
        CHECK(file.getFreeSpace() > 0);
        CHECK(file.getFreeSpace() < file.getFileSize());
    }
}
#endif

TEST_CASE("Test extensible datasets") {
    const std::string file_name("create_extensible_dataset_example.h5");
    const std::string dataset_name("dset");
    constexpr long double t1[3][1] = {{2.0l}, {2.0l}, {4.0l}};
    constexpr long double t2[1][3] = {{4.0l, 8.0l, 6.0l}};

    {
        // Create a new file using the default property lists.
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);

        // Create a dataspace with initial shape and max shape
        DataSpace dataspace = DataSpace({4, 5}, {17, DataSpace::UNLIMITED});

        // Use chunking
        DataSetCreateProps props;
        props.add(Chunking(std::vector<hsize_t>{2, 2}));

        // Create the dataset
        DataSet dataset =
            file.createDataSet(dataset_name, dataspace, AtomicType<long double>(), props);

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
        File file(file_name, File::ReadOnly);

        DataSet dataset_absolute = file.getDataSet("/" + dataset_name);
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
    const std::string file_name("h5_ref_count_test.h5");
    const std::string dataset_name("dset");
    const std::string group_name_1("/group1");
    const std::string group_name_2("/group2");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    std::unique_ptr<DataSet> d1_ptr;
    std::unique_ptr<Group> g_ptr;

    {
        // create group
        Group g1 = file.createGroup(group_name_1);

        // override object
        g1 = file.createGroup(group_name_2);

        // Create the data space for the dataset.
        std::vector<size_t> dims = {10, 10};

        DataSpace dataspace(dims);

        DataSet d1 =
            file.createDataSet(group_name_1 + dataset_name, dataspace, AtomicType<double>());

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
    const std::string file_name("h5_list_test.h5");
    const std::string group_name_core("group_name");
    const std::string group_nested_name("/group_nested");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    {
        // absolute group
        for (int i = 0; i < 2; ++i) {
            std::ostringstream ss;
            ss << "/" << group_name_core << "_" << i;
            file.createGroup(ss.str());
        }

        size_t n_elem = file.getNumberObjects();
        CHECK(2 == n_elem);

        std::vector<std::string> elems = file.listObjectNames();
        CHECK(2 == elems.size());
        std::vector<std::string> reference_elems;
        for (int i = 0; i < 2; ++i) {
            std::ostringstream ss;
            ss << group_name_core << "_" << i;
            reference_elems.push_back(ss.str());
        }

        CHECK(elems == reference_elems);
    }

    {
        file.createGroup(group_nested_name);
        Group g_nest = file.getGroup(group_nested_name);

        for (int i = 0; i < 50; ++i) {
            std::ostringstream ss;
            ss << group_name_core << "_" << i;
            g_nest.createGroup(ss.str());
        }

        size_t n_elem = g_nest.getNumberObjects();
        CHECK(50 == n_elem);

        std::vector<std::string> elems = g_nest.listObjectNames();
        CHECK(50 == elems.size());
        std::vector<std::string> reference_elems;

        for (int i = 0; i < 50; ++i) {
            std::ostringstream ss;
            ss << group_name_core << "_" << i;
            reference_elems.push_back(ss.str());
        }
        // there is no guarantee on the order of the hdf5 index, let's sort it
        // to put them in order
        std::sort(elems.begin(), elems.end());
        std::sort(reference_elems.begin(), reference_elems.end());

        CHECK(elems == reference_elems);
    }
}

TEST_CASE("StringType") {
    SECTION("enshrine-defaults") {
        auto fixed_length = FixedLengthStringType(32, StringPadding::SpacePadded);
        auto variable_length = VariableLengthStringType();

        REQUIRE(fixed_length.getCharacterSet() == CharacterSet::Ascii);
        REQUIRE(variable_length.getCharacterSet() == CharacterSet::Ascii);
    }

    SECTION("fixed-length") {
        auto fixed_length =
            FixedLengthStringType(32, StringPadding::SpacePadded, CharacterSet::Utf8);
        auto string_type = fixed_length.asStringType();

        REQUIRE(string_type.getId() == fixed_length.getId());
        REQUIRE(string_type.getCharacterSet() == CharacterSet::Utf8);
        REQUIRE(string_type.getPadding() == StringPadding::SpacePadded);
        REQUIRE(string_type.getSize() == 32);
        REQUIRE(!string_type.isVariableStr());
        REQUIRE(string_type.isFixedLenStr());
    }

    SECTION("variable-length") {
        auto variable_length = VariableLengthStringType(CharacterSet::Utf8);
        auto string_type = variable_length.asStringType();

        REQUIRE(string_type.getId() == variable_length.getId());
        REQUIRE(string_type.getCharacterSet() == CharacterSet::Utf8);
        REQUIRE(string_type.isVariableStr());
        REQUIRE(!string_type.isFixedLenStr());
    }

    SECTION("atomic") {
        auto atomic = AtomicType<double>();
        REQUIRE_THROWS(atomic.asStringType());
    }
}


TEST_CASE("DataTypeEqualTakeBack") {
    const std::string file_name("h5tutr_dset.h5");
    const std::string dataset_name("dset");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims{10, 1};

    DataSpace dataspace(dims);

    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet<size_t>(dataset_name + "_double", dataspace);

    AtomicType<size_t> s;
    AtomicType<double> d;

    CHECK(s == dataset.getDataType());
    CHECK(d != dataset.getDataType());

    // Test getAddress and expect deprecation warning
    auto addr = dataset.getInfo().getAddress();
    CHECK(addr != 0);
}

TEST_CASE("DataSpaceTest") {
    const std::string file_name("h5tutr_space.h5");
    const std::string dataset_name("dset");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    DataSpace dataspace{std::vector<size_t>{10, 1}};

    // Create a dataset with size_t type
    DataSet dataset = file.createDataSet<size_t>(dataset_name, dataspace);

    DataSpace space = dataset.getSpace();
    DataSpace space2 = dataset.getSpace();
    auto space3 = space.clone();

    // verify space id are different
    CHECK(space.getId() != space2.getId());
    CHECK(space.getId() != space3.getId());

    // verify space id are consistent
    CHECK(space.getDimensions().size() == 2);
    CHECK(space.getDimensions()[0] == 10);
    CHECK(space.getDimensions()[1] == 1);
}

TEST_CASE("DataSpace::getElementCount") {
    SECTION("null") {
        auto space = DataSpace(DataSpace::dataspace_null);
        CHECK(space.getElementCount() == 0);
        CHECK(detail::h5s_get_simple_extent_type(space.getId()) == H5S_NULL);
    }

    SECTION("null named ctor") {
        auto space = DataSpace::Null();
        CHECK(space.getElementCount() == 0);
        CHECK(detail::h5s_get_simple_extent_type(space.getId()) == H5S_NULL);
    }

    SECTION("scalar") {
        auto space = DataSpace(DataSpace::dataspace_scalar);
        CHECK(space.getElementCount() == 1);
        CHECK(detail::h5s_get_simple_extent_type(space.getId()) == H5S_SCALAR);
    }

    SECTION("scalar named ctor") {
        auto space = DataSpace::Scalar();
        CHECK(space.getElementCount() == 1);
        CHECK(detail::h5s_get_simple_extent_type(space.getId()) == H5S_SCALAR);
    }

    SECTION("simple, empty (1D)") {
        auto space = DataSpace(0);
        CHECK(space.getElementCount() == 0);
    }

    SECTION("simple, empty (2D)") {
        auto space = DataSpace(0, 0);
        CHECK(space.getElementCount() == 0);
    }

    SECTION("simple, non-empty (2D)") {
        auto space = DataSpace(2, 3);
        CHECK(space.getElementCount() == 6);
    }

    SECTION("FromCharArrayStrings") {
        char string_array[2][10] = {"123456789", "abcdefghi"};
        auto space = DataSpace::FromCharArrayStrings(string_array);
        CHECK(space.getElementCount() == 2);
    }
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
    const std::string dataset_name("dset");
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
    DataSet dataset = file.createDataSet(dataset_name, vec);
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
    const std::string dataset_name("dset");
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

TEST_CASE("WriteLargeAttribute") {
    std::vector<double> large_attr(16000, 0.0);

    auto fapl = HighFive::FileAccessProps::Default();
    fapl.add(HighFive::FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    HighFive::File file("create_large_attribute.h5", HighFive::File::Truncate, fapl);
    auto gcpl = HighFive::GroupCreateProps::Default();
    gcpl.add(HighFive::AttributePhaseChange(0, 0));

    auto group = file.createGroup("grp", gcpl);
    CHECK_NOTHROW(group.createAttribute("attr", large_attr));
}

TEST_CASE("AttributePhaseChange") {
    auto fapl = HighFive::FileAccessProps::Default();
    fapl.add(HighFive::FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    HighFive::File file("attribute_phase_change.h5", HighFive::File::Truncate, fapl);

    auto gcpl = HighFive::GroupCreateProps::Default();
    gcpl.add(HighFive::AttributePhaseChange(42, 24));

    auto group = file.createGroup("grp", gcpl);

    auto actual = AttributePhaseChange(group.getCreatePropertyList());
    CHECK(actual.min_dense() == 24);
    CHECK(actual.max_compact() == 42);
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

    const std::string dataset_name("dset");

    {
        // Create a new file using the default property lists.
        File file(filename, File::ReadWrite | File::Create | File::Truncate);

        DataSpace d_null(DataSpace::DataspaceType::dataspace_null);

        DataSet d1 = file.createDataSet<double>(dataset_name, d_null);

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
    const std::string dataset_name("dset");
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
        DataSet dataset = file.createDataSet<T>(dataset_name, dataspace, props);

        ContentGenerate<T> generator;
        generate2D(array, x_size, y_size, generator);

        dataset.write(array);

        file.flush();
    }

    // read it back
    {
        File file_read(filename.str(), File::ReadOnly);
        DataSet dataset_read = file_read.getDataSet("/" + dataset_name);

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
    const std::string dataset_name("dset");
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
        DataSet dataset = file.createDataSet<T>(dataset_name, dataspace, props);

        ContentGenerate<T> generator;
        generate2D(array, x_size, y_size, generator);

        dataset.write(array);

        file.flush();
    }

    // read it back
    {
        File file_read(filename.str(), File::ReadOnly);
        DataSet dataset_read = file_read.getDataSet("/" + dataset_name);

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

TEST_CASE("CheckDimensions") {
    // List of dims which can all be one-dimensional.
    std::vector<std::vector<size_t>> test_cases{
        {1ul, 3ul}, {3ul, 1ul}, {1ul, 1ul, 3ul}, {3ul, 1ul, 1ul}, {1ul, 3ul, 1ul}};

    for (const auto& dims: test_cases) {
        auto actual = details::checkDimensions(dims, 1ul);

        INFO("dims = " + details::format_vector(dims) + ", n_dims = 1");
        CHECK(actual);

        INFO("dims = " + details::format_vector(dims) + ", n_dims = 1");
        CHECK(!details::checkDimensions(dims, dims.size() + 1));
    }

    CHECK(details::checkDimensions(std::vector<size_t>{1ul}, 0ul));
    CHECK(details::checkDimensions(std::vector<size_t>{1ul}, 1ul));

    CHECK(!details::checkDimensions(std::vector<size_t>{0ul}, 0ul));
    CHECK(!details::checkDimensions(std::vector<size_t>{2ul}, 0ul));

    CHECK(!details::checkDimensions(std::vector<size_t>{1ul, 2ul, 3ul}, 2ul));
    CHECK(details::checkDimensions(std::vector<size_t>{3ul, 2ul, 1ul}, 2ul));

    CHECK(details::checkDimensions(std::vector<size_t>{1ul, 1ul, 1ul, 1ul}, 1ul));

    CHECK(details::checkDimensions(std::vector<size_t>{}, 0ul));
    CHECK(!details::checkDimensions(std::vector<size_t>{}, 1ul));
    CHECK(!details::checkDimensions(std::vector<size_t>{}, 2ul));
}


TEST_CASE("SqueezeDimensions") {
    SECTION("possible") {
        // List of testcases: the first number is n_dims then the input dimensions
        // and finally the squeezed dimensions.
        std::vector<std::tuple<size_t, std::vector<size_t>, std::vector<size_t>>> test_cases{
            {1ul, {3ul, 1ul}, {3ul}},

            {1ul, {1ul, 1ul, 1ul}, {1ul}},

            {1ul, {1ul, 3ul, 1ul}, {3ul}},

            {1ul, {3ul, 1ul, 1ul}, {3ul}},
            {2ul, {3ul, 1ul, 1ul}, {3ul, 1ul}},
            {3ul, {3ul, 1ul, 1ul}, {3ul, 1ul, 1ul}},

            {3ul, {2ul, 1ul, 3ul}, {2ul, 1ul, 3ul}}};

        for (const auto& tc: test_cases) {
            auto n_dim_requested = std::get<0>(tc);
            auto dims = std::get<1>(tc);
            auto expected = std::get<2>(tc);
            auto actual = details::squeezeDimensions(dims, n_dim_requested);

            CHECK(actual == expected);
        }
    }

    SECTION("impossible") {
        // List of testcases: the first number is n_dims then the input dimensions
        // and finally the squeezed dimensions.
        std::vector<std::tuple<size_t, std::vector<size_t>>> test_cases{{1ul, {1ul, 2ul, 3ul}},
                                                                        {2ul, {1ul, 2ul, 3ul, 1ul}},

                                                                        {1ul, {2ul, 1ul, 3ul}},
                                                                        {2ul, {2ul, 1ul, 3ul}}};

        for (const auto& tc: test_cases) {
            auto n_dim_requested = std::get<0>(tc);
            auto dims = std::get<1>(tc);

            CHECK_THROWS(details::squeezeDimensions(dims, n_dim_requested));
        }
    }
}

void check_broadcast_1d(HighFive::File& file,
                        const std::vector<size_t> dims,
                        const std::string& dataset_name) {
    // This checks that:
    //   - we can write 1D array into 2D dataset.
    //   - we can read 2D dataset into a 1D array.
    std::vector<double> input_data{5.0, 6.0, 7.0};


    DataSpace dataspace(dims);
    DataSet dataset = file.createDataSet(dataset_name, dataspace, AtomicType<double>());

    dataset.write(input_data);

    {
        std::vector<double> read_back;
        dataset.read(read_back);

        CHECK(read_back == input_data);
    }

    {
        auto read_back = dataset.read<std::vector<double>>();
        CHECK(read_back == input_data);
    }
}

// Broadcasting is supported
TEST_CASE("ReadInBroadcastDims") {
    const std::string file_name("h5_broadcast_dset.h5");
    const std::string dataset_name("dset");

    // Create a new file using the default property lists.
    File file(file_name, File::Truncate);

    SECTION("one-dimensional (1, 3)") {
        check_broadcast_1d(file, {1, 3}, dataset_name + "_a");
    }

    SECTION("one-dimensional (3, 1)") {
        check_broadcast_1d(file, {3, 1}, dataset_name + "_b");
    }

    SECTION("two-dimensional (2, 3, 1)") {
        std::vector<size_t> dims{2, 3, 1};
        std::vector<std::vector<double>> input_data_2d{{2.0, 3.0, 4.0}, {10.0, 11.0, 12.0}};

        DataSpace dataspace(dims);
        DataSet dataset = file.createDataSet(dataset_name + "_c", dataspace, AtomicType<double>());

        dataset.write(input_data_2d);

        auto check = [](const std::vector<std::vector<double>>& lhs,
                        const std::vector<std::vector<double>>& rhs) {
            CHECK(lhs.size() == rhs.size());
            for (size_t i = 0; i < rhs.size(); ++i) {
                CHECK(lhs[i].size() == rhs[i].size());

                for (size_t j = 0; j < rhs[i].size(); ++j) {
                    CHECK(lhs[i][j] == rhs[i][j]);
                }
            }
        };

        {
            std::vector<std::vector<double>> read_back;
            dataset.read(read_back);

            check(read_back, input_data_2d);
        }

        {
            auto read_back = dataset.read<std::vector<std::vector<double>>>();
            check(read_back, input_data_2d);
        }
    }

    SECTION("one-dimensional fixed length string") {
        std::vector<size_t> dims{1, 1, 2};
        char input_data[2] = "a";

        DataSpace dataspace(dims);
        DataSet dataset = file.createDataSet(dataset_name + "_d", dataspace, AtomicType<char>());
        dataset.write(input_data);

        {
            char read_back[2];
            dataset.read(read_back);

            CHECK(read_back[0] == 'a');
            CHECK(read_back[1] == '\0');
        }
    }
}


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

#ifdef H5_USE_BOOST
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


#ifdef H5_USE_EIGEN
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

struct ReadWriteAttribute {
    template <class Container>
    static void create(HighFive::File& file, const std::string& name, const Container& container) {
        file.createAttribute(name, container);
    }

    static HighFive::Attribute get(HighFive::File& file, const std::string& name) {
        return file.getAttribute(name);
    }
};

struct ReadWriteDataSet {
    template <class Container>
    static void create(HighFive::File& file, const std::string& name, const Container& container) {
        file.createDataSet(name, container);
    }

    static HighFive::DataSet get(HighFive::File& file, const std::string& name) {
        return file.getDataSet(name);
    }
};

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

        ReadWriteInterface::get(file, dataset_name).read(output_data);
        check_empty_dimensions(output_data, {0ul});
    }

    SECTION("read; pre-allocated (empty)") {
        auto output_data = CreateContainer::create(dims);
        ReadWriteInterface::get(file, dataset_name).read(output_data);

        check_empty_dimensions(output_data, dims);
    }

    SECTION("read; pre-allocated (oversized)") {
        auto oversize_dims = std::vector<size_t>(dims.size(), 2ul);
        auto output_data = CreateContainer::create(oversize_dims);
        ReadWriteInterface::get(file, dataset_name).read(output_data);

        check_empty_dimensions(output_data, dims);
    }

    SECTION("read; auto-allocated") {
        auto output_data =
            ReadWriteInterface::get(file, dataset_name).template read<container_type>();
        check_empty_dimensions(output_data, dims);
    }
}

template <class CreateContainer>
void check_empty_dataset(const std::vector<size_t>& dims) {
    check_empty_read_write_cycle<ReadWriteDataSet, CreateContainer>(dims);
}

template <class CreateContainer>
void check_empty_attribute(const std::vector<size_t>& dims) {
    check_empty_read_write_cycle<ReadWriteAttribute, CreateContainer>(dims);
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

#ifdef H5_USE_EIGEN
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

#ifdef H5_USE_BOOST
    SECTION("boost::multi_array") {
        check_empty_everything<CreateEmptyBoostMultiArray<ndim>>(dims);
    }
#endif

#ifdef H5_USE_EIGEN
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

TEST_CASE("HighFiveRecursiveGroups") {
    const std::string file_name("h5_ds_exist.h5");
    const std::string group_1("group1");
    const std::string group_2("group2");
    const std::string ds_path = group_1 + "/" + group_2;
    const std::string ds_name = "ds";

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    CHECK(file.getName() == file_name);

    // Without parents creating both groups will fail
    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.createGroup(ds_path, false), std::exception);
    }
    Group g2 = file.createGroup(ds_path);

    std::vector<double> some_data{5.0, 6.0, 7.0};
    g2.createDataSet(ds_name, some_data);

    CHECK(file.exist(group_1));

    Group g1 = file.getGroup(group_1);
    CHECK(g1.exist(group_2));

    // checks with full path
    CHECK(file.exist(ds_path));
    CHECK(file.exist(ds_path + "/" + ds_name));

    // Check with wrong middle path (before would raise Exception)
    CHECK(!file.exist(std::string("blabla/group2")));

    // Using root slash
    CHECK(file.exist(std::string("/") + ds_path));

    // Check unlink with existing group
    CHECK(g1.exist(group_2));
    g1.unlink(group_2);
    CHECK(!g1.exist(group_2));

    // Check unlink with non-existing group
    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(g1.unlink("x"), HighFive::GroupException);
    }
}

TEST_CASE("HighFiveInspect") {
    const std::string file_name("group_info.h5");
    const std::string group_1("group1");
    const std::string ds_name = "ds";

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    Group g = file.createGroup(group_1);

    std::vector<double> some_data{5.0, 6.0, 7.0};
    g.createDataSet(ds_name, some_data);

    CHECK(file.getLinkType(group_1) == LinkType::Hard);

    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.getLinkType("x"), HighFive::GroupException);
    }

    CHECK(file.getObjectType(group_1) == ObjectType::Group);
    CHECK(file.getObjectType(group_1 + "/" + ds_name) == ObjectType::Dataset);
    CHECK(g.getObjectType(ds_name) == ObjectType::Dataset);

    {
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.getObjectType(ds_name), HighFive::GroupException);
    }

    // Data type
    auto ds = g.getDataSet(ds_name);
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
    const std::string file_name("softlinks.h5");
    const std::string ds_path("/hard_link/dataset");
    const std::string link_path("/soft_link/to_ds");
    const std::vector<int> data{11, 22, 33};

    {
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);
        auto dset = file.createDataSet(ds_path, data);
        file.createSoftLink(link_path, dset);
    }

    {
        File file(file_name, File::ReadWrite);
        std::vector<int> data_out;
        file.getDataSet(link_path).read(data_out);
        CHECK(data == data_out);
    }

    {
        const std::string EXTERNAL_LINK_PATH("/external_link/to_ds");
        File file2("link_external_to.h5", File::ReadWrite | File::Create | File::Truncate);
        file2.createExternalLink(EXTERNAL_LINK_PATH, file_name, ds_path);

        std::vector<int> data_out;
        file2.getDataSet(EXTERNAL_LINK_PATH).read(data_out);
        CHECK(data == data_out);
    }
}

TEST_CASE("HighFiveHardLinks Dataset (create intermediate)") {
    const std::string file_name("hardlinks_dataset_intermiate.h5");
    const std::string ds_path("/group/dataset");
    const std::string ds_link_path("/alternate/dataset");
    const std::vector<int> data{12, 24, 36};

    {
        File file(file_name, File::Truncate);
        auto dset = file.createDataSet(ds_path, data);
        file.createHardLink(ds_link_path, dset);
        file.unlink(ds_path);
    }

    {
        File file(file_name, File::ReadWrite);
        auto data_out = file.getDataSet(ds_link_path).read<std::vector<int>>();
        CHECK(data == data_out);
    }
}

TEST_CASE("HighFiveHardLinks Dataset (relative paths)") {
    const std::string file_name("hardlinks_dataset_relative.h5");
    const std::string ds_path("/group/dataset");
    const std::string ds_link_path("/alternate/dataset");
    const std::vector<int> data{12, 24, 36};

    {
        File file(file_name, File::Truncate);
        auto dset = file.createDataSet(ds_path, data);

        auto alternate = file.createGroup("/alternate");
        alternate.createHardLink("dataset", dset);
        file.unlink(ds_path);
    }

    {
        File file(file_name, File::ReadWrite);
        auto data_out = file.getDataSet(ds_link_path).read<std::vector<int>>();
        CHECK(data == data_out);
    }
}

TEST_CASE("HighFiveHardLinks Group") {
    const std::string file_name("hardlinks_group.h5");
    const std::string group_path("/group");
    const std::string ds_name("dataset");
    const std::string group_link_path("/alternate");
    const std::vector<int> data{12, 24, 36};

    {
        File file(file_name, File::Truncate);
        auto dset = file.createDataSet(group_path + "/" + ds_name, data);
        auto group = file.getGroup(group_path);
        file.createHardLink(group_link_path, group);
        file.unlink(group_path);
    }

    {
        File file(file_name, File::ReadWrite);
        auto data_out = file.getDataSet(group_link_path + "/" + ds_name).read<std::vector<int>>();
        CHECK(data == data_out);
    }
}

TEST_CASE("HighFiveRename") {
    File file("h5_rename.h5", File::ReadWrite | File::Create | File::Truncate);

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
    File file("h5_rename_relative.h5", File::ReadWrite | File::Create | File::Truncate);
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

TEST_CASE("HighFivePropertyObjectsQuirks") {
    auto pl1 = LinkCreateProps::Default();
    auto pl2 = pl1;
    // As usual shallow copying semantics apply:
    CHECK(pl1.getId() == pl2.getId());

    // Then one adds something and the link is broken:
    pl2.add(CreateIntermediateGroup(true));
    CHECK(pl1.getId() == H5P_DEFAULT);

    // but once it's not a "special" value, regular shallow copy semantic
    // return:
    auto pl3 = pl2;
    pl3.add(CreateIntermediateGroup(false));
    CHECK(pl3.getId() == pl2.getId());
}

TEST_CASE("HighFiveLinkCreationOrderProperty") {
    {  // For file
        const std::string file_name("h5_keep_creation_order_file.h5");
        FileCreateProps keepCreationOrder{};
        keepCreationOrder.add(LinkCreationOrder(CreationOrder::Tracked | CreationOrder::Indexed));

        File file(file_name, File::ReadWrite | File::Create | File::Truncate, keepCreationOrder);
        file.createGroup("1");
        file.createGroup("2");
        file.createGroup("10");

        CHECK(file.listObjectNames(IndexType::CRT_ORDER) ==
              std::vector<std::string>{"1", "2", "10"});
        CHECK(file.listObjectNames(IndexType::NAME) == std::vector<std::string>{"1", "10", "2"});

        auto fcpl = file.getCreatePropertyList();
        LinkCreationOrder linkCreationOrder(fcpl);
        CHECK((linkCreationOrder.getFlags() & CreationOrder::Tracked) != 0);
        CHECK((linkCreationOrder.getFlags() & CreationOrder::Indexed) != 0);
    }
    {  // For groups
        const std::string file_name("h5_keep_creation_order_group.h5");
        GroupCreateProps keepCreationOrder{};
        keepCreationOrder.add(LinkCreationOrder(CreationOrder::Tracked | CreationOrder::Indexed));

        File file(file_name, File::ReadWrite | File::Create | File::Truncate);
        auto group = file.createGroup("group_crt", keepCreationOrder);
        group.createGroup("1");
        group.createGroup("2");
        group.createGroup("10");

        CHECK(group.listObjectNames(IndexType::CRT_ORDER) ==
              std::vector<std::string>{"1", "2", "10"});
        CHECK(group.listObjectNames(IndexType::NAME) == std::vector<std::string>{"1", "10", "2"});

        auto group2 = file.createGroup("group_name");
        group2.createGroup("1");
        group2.createGroup("2");
        group2.createGroup("10");

        CHECK(group2.listObjectNames() == std::vector<std::string>{"1", "10", "2"});

        {
            auto gcpl = group.getCreatePropertyList();
            LinkCreationOrder linkCreationOrder(gcpl);
            CHECK((linkCreationOrder.getFlags() & CreationOrder::Tracked) != 0);
            CHECK((linkCreationOrder.getFlags() & CreationOrder::Indexed) != 0);
        }
        {
            auto gcpl = group2.getCreatePropertyList();
            LinkCreationOrder linkCreationOrder(gcpl);
            CHECK((linkCreationOrder.getFlags() & CreationOrder::Tracked) == 0);
            CHECK((linkCreationOrder.getFlags() & CreationOrder::Indexed) == 0);
        }
    }
}

TEST_CASE("DirectWriteBool") {
    SECTION("Basic compatibility") {
        CHECK(sizeof(bool) == sizeof(details::Boolean));

        auto n_bytes = 2 * sizeof(details::Boolean);

        auto* const enum_ptr = (details::Boolean*) malloc(n_bytes);
        memset(enum_ptr, 187, n_bytes);
        enum_ptr[0] = details::Boolean::HighFiveTrue;
        enum_ptr[1] = details::Boolean::HighFiveFalse;

        auto* const bool_ptr = (bool*) malloc(n_bytes);
        memset(bool_ptr, 68, n_bytes);
        bool_ptr[0] = true;
        bool_ptr[1] = false;

        CHECK(std::memcmp(bool_ptr, enum_ptr, n_bytes) == 0);

        free(enum_ptr);
        free(bool_ptr);
    }

    auto file = File("rw_bool_from_ptr.h5", File::Truncate);

    size_t n = 4;
    bool* expected = new bool[n];
    bool* actual = new bool[n];

    for (size_t i = 0; i < n; ++i) {
        expected[i] = i % 2 == 0;
    }

    auto dataspace = DataSpace{n};
    auto datatype = create_datatype<bool>();

    SECTION("WriteReadCycleAttribute") {
        auto attr = file.createAttribute("attr", dataspace, datatype);
        attr.write_raw(expected);
        attr.read(actual);

        for (size_t i = 0; i < n; ++i) {
            REQUIRE(expected[i] == actual[i]);
        }
    }

    SECTION("WriteReadCycleDataSet") {
        auto dset = file.createAttribute("dset", dataspace, datatype);
        dset.write_raw(expected);
        dset.read(actual);

        for (size_t i = 0; i < n; ++i) {
            REQUIRE(expected[i] == actual[i]);
        }
    }

    delete[] expected;
    delete[] actual;
}


class ForwardToAttribute {
  public:
    ForwardToAttribute(const HighFive::File& file)
        : _file(file) {}

    template <class T>
    HighFive::Attribute create(const std::string& name, const T& value) {
        return _file.createAttribute(name, value);
    }

    HighFive::Attribute create(const std::string& name,
                               const HighFive::DataSpace filespace,
                               const HighFive::DataType& datatype) {
        return _file.createAttribute(name, filespace, datatype);
    }

    HighFive::Attribute get(const std::string& name) {
        return _file.getAttribute(name);
    }

  private:
    HighFive::File _file;
};

class ForwardToDataSet {
  public:
    ForwardToDataSet(const HighFive::File& file)
        : _file(file) {}

    template <class T>
    HighFive::DataSet create(const std::string& name, const T& value) {
        return _file.createDataSet(name, value);
    }

    HighFive::DataSet create(const std::string& name,
                             const HighFive::DataSpace filespace,
                             const HighFive::DataType& datatype) {
        return _file.createDataSet(name, filespace, datatype);
    }

    HighFive::DataSet get(const std::string& name) {
        return _file.getDataSet(name);
    }

  private:
    HighFive::File _file;
};

template <class Proxy>
void check_single_string(Proxy proxy, size_t string_length) {
    auto value = std::string(string_length, 'o');
    auto dataspace = DataSpace::From(value);

    auto n_chars = value.size() + 1;
    auto n_chars_overlength = n_chars + 10;
    auto fixed_length = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
    auto overlength_nullterm = FixedLengthStringType(n_chars_overlength,
                                                     StringPadding::NullTerminated);
    auto overlength_nullpad = FixedLengthStringType(n_chars_overlength, StringPadding::NullPadded);
    auto overlength_spacepad = FixedLengthStringType(n_chars_overlength,
                                                     StringPadding::SpacePadded);
    auto variable_length = VariableLengthStringType();

    SECTION("automatic") {
        proxy.create("auto", value);
        REQUIRE(proxy.get("auto").template read<std::string>() == value);
    }

    SECTION("fixed length") {
        proxy.create("fixed", dataspace, fixed_length).write(value);
        REQUIRE(proxy.get("fixed").template read<std::string>() == value);
    }

    SECTION("overlength null-terminated") {
        proxy.create("overlength_nullterm", dataspace, overlength_nullterm).write(value);
        REQUIRE(proxy.get("overlength_nullterm").template read<std::string>() == value);
    }

    SECTION("overlength null-padded") {
        proxy.create("overlength_nullpad", dataspace, overlength_nullpad).write(value);
        auto expected = std::string(n_chars_overlength, '\0');
        expected.replace(0, value.size(), value.data());
        REQUIRE(proxy.get("overlength_nullpad").template read<std::string>() == expected);
    }

    SECTION("overlength space-padded") {
        proxy.create("overlength_spacepad", dataspace, overlength_spacepad).write(value);
        auto expected = std::string(n_chars_overlength, ' ');
        expected.replace(0, value.size(), value.data());
        REQUIRE(proxy.get("overlength_spacepad").template read<std::string>() == expected);
    }

    SECTION("variable length") {
        proxy.create("variable", dataspace, variable_length).write(value);
        REQUIRE(proxy.get("variable").template read<std::string>() == value);
    }
}

template <class Proxy>
void check_multiple_string(Proxy proxy, size_t string_length) {
    using value_t = std::vector<std::string>;
    auto value = value_t{std::string(string_length, 'o'), std::string(string_length, 'x')};

    auto dataspace = DataSpace::From(value);

    auto string_overlength = string_length + 10;
    auto onpoint_nullpad = FixedLengthStringType(string_length, StringPadding::NullPadded);
    auto onpoint_spacepad = FixedLengthStringType(string_length, StringPadding::SpacePadded);

    auto overlength_nullterm = FixedLengthStringType(string_overlength,
                                                     StringPadding::NullTerminated);
    auto overlength_nullpad = FixedLengthStringType(string_overlength, StringPadding::NullPadded);
    auto overlength_spacepad = FixedLengthStringType(string_overlength, StringPadding::SpacePadded);
    auto variable_length = VariableLengthStringType();

    auto check = [](const value_t actual, const value_t& expected) {
        REQUIRE(actual.size() == expected.size());
        for (size_t i = 0; i < actual.size(); ++i) {
            REQUIRE(actual[i] == expected[i]);
        }
    };

    SECTION("automatic") {
        proxy.create("auto", value);
        check(proxy.get("auto").template read<value_t>(), value);
    }

    SECTION("variable length") {
        proxy.create("variable", dataspace, variable_length).write(value);
        check(proxy.get("variable").template read<value_t>(), value);
    }

    auto make_padded_reference = [&](char pad, size_t n) {
        auto expected = std::vector<std::string>(value.size(), std::string(n, pad));
        for (size_t i = 0; i < value.size(); ++i) {
            expected[i].replace(0, value[i].size(), value[i].data());
        }

        return expected;
    };

    auto check_fixed_length = [&](const std::string& label, size_t length) {
        SECTION(label + " null-terminated") {
            auto datatype = FixedLengthStringType(length + 1, StringPadding::NullTerminated);
            proxy.create(label + "_nullterm", dataspace, datatype).write(value);
            check(proxy.get(label + "_nullterm").template read<value_t>(), value);
        }

        SECTION(label + " null-padded") {
            auto datatype = FixedLengthStringType(length, StringPadding::NullPadded);
            proxy.create(label + "_nullpad", dataspace, datatype).write(value);
            auto expected = make_padded_reference('\0', length);
            check(proxy.get(label + "_nullpad").template read<value_t>(), expected);
        }

        SECTION(label + " space-padded") {
            auto datatype = FixedLengthStringType(length, StringPadding::SpacePadded);
            proxy.create(label + "_spacepad", dataspace, datatype).write(value);
            auto expected = make_padded_reference(' ', length);
            check(proxy.get(label + "_spacepad").template read<value_t>(), expected);
        }
    };

    check_fixed_length("onpoint", string_length);
    check_fixed_length("overlength", string_length + 5);


    SECTION("underlength null-terminated") {
        auto datatype = FixedLengthStringType(string_length, StringPadding::NullTerminated);
        REQUIRE_THROWS(proxy.create("underlength_nullterm", dataspace, datatype).write(value));
    }

    SECTION("underlength nullpad") {
        auto datatype = FixedLengthStringType(string_length - 1, StringPadding::NullPadded);
        REQUIRE_THROWS(proxy.create("underlength_nullpad", dataspace, datatype).write(value));
    }

    SECTION("underlength spacepad") {
        auto datatype = FixedLengthStringType(string_length - 1, StringPadding::NullTerminated);
        REQUIRE_THROWS(proxy.create("underlength_spacepad", dataspace, datatype).write(value));
    }
}

TEST_CASE("HighFiveSTDString (dataset, single, short)") {
    File file("std_string_dataset_single_short.h5", File::Truncate);
    check_single_string(ForwardToDataSet(file), 3);
}

TEST_CASE("HighFiveSTDString (attribute, single, short)") {
    File file("std_string_attribute_single_short.h5", File::Truncate);
    check_single_string(ForwardToAttribute(file), 3);
}

TEST_CASE("HighFiveSTDString (dataset, single, long)") {
    File file("std_string_dataset_single_long.h5", File::Truncate);
    check_single_string(ForwardToDataSet(file), 256);
}

TEST_CASE("HighFiveSTDString (attribute, single, long)") {
    File file("std_string_attribute_single_long.h5", File::Truncate);
    check_single_string(ForwardToAttribute(file), 256);
}

TEST_CASE("HighFiveSTDString (dataset, multiple, short)") {
    File file("std_string_dataset_multiple_short.h5", File::Truncate);
    check_multiple_string(ForwardToDataSet(file), 3);
}

TEST_CASE("HighFiveSTDString (attribute, multiple, short)") {
    File file("std_string_attribute_multiple_short.h5", File::Truncate);
    check_multiple_string(ForwardToAttribute(file), 3);
}

TEST_CASE("HighFiveSTDString (dataset, multiple, long)") {
    File file("std_string_dataset_multiple_long.h5", File::Truncate);
    check_multiple_string(ForwardToDataSet(file), 256);
}

TEST_CASE("HighFiveSTDString (attribute, multiple, long)") {
    File file("std_string_attribute_multiple_long.h5", File::Truncate);
    check_multiple_string(ForwardToAttribute(file), 256);
}

TEST_CASE("HighFiveFixedString") {
    const std::string file_name("array_atomic_types.h5");
    const std::string group_1("group1");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    char raw_strings[][10] = {"abcd", "1234"};

    /// This will not compile - only char arrays - hits static_assert with a nice
    /// error
    // file.createDataSet<int[10]>(ds_name, DataSpace(2)));

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

    {
        // Direct way of writing `std::string` as a fixed length
        // HDF5 string.

        std::string value = "foo";
        auto n_chars = value.size() + 1;

        auto datatype = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
        auto dataspace = DataSpace(1);

        auto ds = file.createDataSet("ds8", dataspace, datatype);
        ds.write_raw(value.data(), datatype);

        {
            // Due to missing non-const overload of `data()` until C++17 we'll
            // read into something else instead (don't forget the '\0').
            auto expected = std::vector<char>(n_chars, '!');
            ds.read(expected.data(), datatype);

            CHECK(expected.size() == value.size() + 1);
            for (size_t i = 0; i < value.size(); ++i) {
                REQUIRE(expected[i] == value[i]);
            }
        }

#if HIGHFIVE_CXX_STD >= 17
        {
            auto expected = std::string(value.size(), '-');
            ds.read(expected.data(), datatype);

            REQUIRE(expected == value);
        }
#endif
    }

    {
        size_t n_chars = 4;
        size_t n_strings = 2;

        std::vector<char> value(n_chars * n_strings, '!');

        auto datatype = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
        auto dataspace = DataSpace(n_strings);

        auto ds = file.createDataSet("ds9", dataspace, datatype);
        ds.write_raw(value.data(), datatype);

        auto expected = std::vector<char>(value.size(), '-');
        ds.read(expected.data(), datatype);

        CHECK(expected.size() == value.size());
        for (size_t i = 0; i < value.size(); ++i) {
            REQUIRE(expected[i] == value[i]);
        }
    }
}

template <size_t N>
static void check_fixed_len_string_array_contents(const FixedLenStringArray<N>& array,
                                                  const std::vector<std::string>& expected) {
    REQUIRE(array.size() == expected.size());

    for (size_t i = 0; i < array.size(); ++i) {
        CHECK(array[i] == expected[i]);
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

    SECTION("create from std::vector (onpoint)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<4>(expected);
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from std::vector (oversized)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<8>(expected);
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from pointers (onpoint)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<4>(expected.data(), expected.data() + expected.size());
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from pointers (oversized)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<8>(expected.data(), expected.data() + expected.size());
        check_fixed_len_string_array_contents(actual, expected);
    }


    SECTION("create from std::initializer_list (onpoint)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<4>{"000", "111"};
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from std::initializer_list (oversized)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<8>{"000", "111"};
        check_fixed_len_string_array_contents(actual, expected);
    }

    // manipulate FixedLenStringArray with std::copy
    SECTION("compatible with std::copy") {
        const fixed_array_t arr1{"0000000", "1111111"};
        fixed_array_t arr2{"0000000", "1111111"};
        std::copy(arr1.begin(), arr1.end(), std::back_inserter(arr2));
        CHECK(arr2.size() == 4);
    }

    SECTION("compatible with std::transform") {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::transform(arr1.begin(), arr1.end(), std::back_inserter(arr), increment_string);
        }
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("1111111"));
        CHECK(arr[1] == std::string("2222222"));
    }

    SECTION("compatible with std::transform (reverse iterator)") {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::copy(arr1.rbegin(), arr1.rend(), std::back_inserter(arr));
        }
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("1111111"));
        CHECK(arr[1] == std::string("0000000"));
    }

    SECTION("compatible with std::remove_copy_if") {
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
    const std::string file_name("fixed_array_attr.h5");
    // Create a new file using the default property lists.
    {
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);
        FixedLenStringArray<10> arr{"Hello", "world"};
        file.createAttribute("str", arr);
    }
    // Re-read it
    {
        File file(file_name);
        FixedLenStringArray<8> arr;  // notice the output strings can be smaller
        file.getAttribute("str").read(arr);
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("Hello"));
        CHECK(arr[1] == std::string("world"));
    }
}

TEST_CASE("HighFiveReference") {
    const std::string file_name("h5_ref_test.h5");
    const std::string dataset1_name("dset1");
    const std::string dataset2_name("dset2");
    const std::string group_name("/group1");
    const std::string refgroup_name("/group2");
    const std::string refdataset_name("dset2");

    ContentGenerate<double> generator;
    std::vector<double> vec1(4);
    std::vector<double> vec2(4);
    std::generate(vec1.begin(), vec1.end(), generator);
    std::generate(vec2.begin(), vec2.end(), generator);
    {
        // Create a new file using the default property lists.
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);

        // create group
        Group g1 = file.createGroup(group_name);

        // create datasets and write some data
        DataSet dataset1 = g1.createDataSet(dataset1_name, vec1);
        DataSet dataset2 = g1.createDataSet(dataset2_name, vec2);

        // create group to hold reference
        Group refgroup = file.createGroup(refgroup_name);

        // create the references and write them into a new dataset inside refgroup
        auto references = std::vector<Reference>({{g1, dataset1}, {file, g1}});
        DataSet ref_ds = refgroup.createDataSet(refdataset_name, references);
    }
    // read it back
    {
        File file(file_name, File::ReadOnly);
        Group refgroup = file.getGroup(refgroup_name);

        DataSet refdataset = refgroup.getDataSet(refdataset_name);
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
        DataSet data_ds2 = group.getDataSet(dataset2_name);
        std::vector<double> rdata2;
        data_ds2.read(rdata2);
        for (size_t i = 0; i < rdata2.size(); ++i) {
            CHECK(rdata2[i] == vec2[i]);
        }
    }
}

TEST_CASE("HighFiveReadWriteConsts") {
    const std::string file_name("3d_dataset_from_flat.h5");
    const std::string dataset_name("dset");
    const std::array<std::size_t, 3> DIMS{3, 3, 3};
    using datatype = int;

    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    DataSpace dataspace = DataSpace(DIMS);

    DataSet dataset = file.createDataSet<datatype>(dataset_name, dataspace);
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
    const std::string ds_name = "ds";
    file.createDataSet(ds_name + test_flavor, vec_input).write(vec_input);
    file.getDataSet(ds_name + test_flavor).read(vec_output);
    CHECK(vec_input == vec_output);
}

TEST_CASE("HighFiveEigen") {
    const std::string file_name("test_eigen.h5");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    std::string ds_name_flavor;

    // std::vector<of vector <of POD>>
    {
        ds_name_flavor = "VectorOfVectorOfPOD";
        std::vector<std::vector<float>> vec_in{{5.0f, 6.0f, 7.0f},
                                               {5.1f, 6.1f, 7.1f},
                                               {5.2f, 6.2f, 7.2f}};
        std::vector<std::vector<float>> vec_out;
        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
    }

    // std::vector<Eigen::Vector3d>
    {
        ds_name_flavor = "VectorOfEigenVector3d";
        std::vector<Eigen::Vector3d> vec_in{{5.0, 6.0, 7.0}, {7.0, 8.0, 9.0}};
        std::vector<Eigen::Vector3d> vec_out;
        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
    }

    // Eigen Vector2d
    {
        ds_name_flavor = "EigenVector2d";
        Eigen::Vector2d vec_in{5.0, 6.0};
        Eigen::Vector2d vec_out;

        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
    }

    // Eigen Matrix
    {
        ds_name_flavor = "EigenMatrix";
        Eigen::Matrix<double, 3, 3> vec_in;
        vec_in << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        Eigen::Matrix<double, 3, 3> vec_out;

        CHECK_THROWS(test_eigen_vec(file, ds_name_flavor, vec_in, vec_out));
    }

    // Eigen MatrixXd
    {
        ds_name_flavor = "EigenMatrixXd";
        Eigen::MatrixXd vec_in = 100. * Eigen::MatrixXd::Random(20, 5);
        Eigen::MatrixXd vec_out(20, 5);

        CHECK_THROWS(test_eigen_vec(file, ds_name_flavor, vec_in, vec_out));
    }

    // std::vector<of EigenMatrixXd>
    {
        ds_name_flavor = "VectorEigenMatrixXd";

        Eigen::MatrixXd m1 = 100. * Eigen::MatrixXd::Random(20, 5);
        Eigen::MatrixXd m2 = 100. * Eigen::MatrixXd::Random(20, 5);
        std::vector<Eigen::MatrixXd> vec_in;
        vec_in.push_back(m1);
        vec_in.push_back(m2);
        std::vector<Eigen::MatrixXd> vec_out(2, Eigen::MatrixXd::Zero(20, 5));

        CHECK_THROWS(test_eigen_vec(file, ds_name_flavor, vec_in, vec_out));
    }

#ifdef H5_USE_BOOST
    // boost::multi_array<of EigenVector3f>
    {
        ds_name_flavor = "BMultiEigenVector3f";

        boost::multi_array<Eigen::Vector3f, 3> vec_in(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_in[i][j][k] = Eigen::Vector3f::Random(3);
                }
            }
        }
        boost::multi_array<Eigen::Vector3f, 3> vec_out(boost::extents[3][2][2]);

        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
    }

    // boost::multi_array<of EigenMatrixXd>
    {
        ds_name_flavor = "BMultiEigenMatrixXd";

        boost::multi_array<Eigen::MatrixXd, 3> vec_in(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_in[i][j][k] = Eigen::MatrixXd::Random(3, 3);
                }
            }
        }
        boost::multi_array<Eigen::MatrixXd, 3> vec_out(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_out[i][j][k] = Eigen::MatrixXd::Zero(3, 3);
                }
            }
        }

        CHECK_THROWS(test_eigen_vec(file, ds_name_flavor, vec_in, vec_out));
    }

#endif
}
#endif

TEST_CASE("Logging") {
    struct TestLogger {
        LogSeverity last_log_severity = LogSeverity(11);
        std::string last_message = "---";

        void operator()(LogSeverity log_severity,
                        const std::string& message,
                        const std::string&,
                        int) {
            std::cout << "Something got logged !!!" << message << " " << to_string(log_severity)
                      << std::endl;
            last_log_severity = log_severity;
            last_message = message;
        }
    };

    auto test_logger = TestLogger();

    register_logging_callback(
        [&test_logger](LogSeverity log_severity,
                       const std::string& message,
                       const std::string& file,
                       int line) { test_logger(log_severity, message, file, line); });

    auto check = [&test_logger](bool should_log, const auto& message, LogSeverity log_severity) {
        if (should_log && (HIGHFIVE_LOG_LEVEL <= int(log_severity))) {
            REQUIRE(test_logger.last_message == message);
            REQUIRE(test_logger.last_log_severity == log_severity);
        } else {
            REQUIRE(test_logger.last_message != message);
            REQUIRE(test_logger.last_log_severity != log_severity);
        }
    };

    SECTION("LOG_DEBUG") {
        auto message = "Debug!";
        HIGHFIVE_LOG_DEBUG(message);
        check(true, message, LogSeverity::Debug);
    }

    SECTION("LOG_DEBUG_IF true") {
        auto message = "DEBUG_IF true!";
        HIGHFIVE_LOG_DEBUG_IF(true, message);
        check(true, message, LogSeverity::Debug);
    }

    SECTION("LOG_DEBUG_IF false") {
        auto message = "DEBUG_IF false!";
        HIGHFIVE_LOG_DEBUG_IF(false, message);
        check(false, message, LogSeverity::Debug);
    }

    SECTION("LOG_INFO") {
        auto message = "Info!";
        HIGHFIVE_LOG_INFO(message);
        check(true, message, LogSeverity::Info);
    }

    SECTION("LOG_INFO_IF true") {
        auto message = "INFO_IF true!";
        HIGHFIVE_LOG_INFO_IF(true, message);
        check(true, message, LogSeverity::Info);
    }

    SECTION("LOG_INFO_IF false") {
        auto message = "INFO_IF false!";
        HIGHFIVE_LOG_INFO_IF(false, message);
        check(false, message, LogSeverity::Info);
    }

    SECTION("LOG_WARN") {
        auto message = "Warn!";
        HIGHFIVE_LOG_WARN(message);
        check(true, message, LogSeverity::Warn);
    }

    SECTION("LOG_WARN_IF true") {
        auto message = "WARN_IF true!";
        HIGHFIVE_LOG_WARN_IF(true, message);
        check(true, message, LogSeverity::Warn);
    }

    SECTION("LOG_WARN_IF false") {
        auto message = "WARN_IF false!";
        HIGHFIVE_LOG_WARN_IF(false, message);
        check(false, message, LogSeverity::Warn);
    }

    SECTION("LOG_ERROR") {
        auto message = "Error!";
        HIGHFIVE_LOG_ERROR(message);
        check(true, message, LogSeverity::Error);
    }

    SECTION("LOG_ERROR_IF true") {
        auto message = "ERROR_IF true!";
        HIGHFIVE_LOG_ERROR_IF(true, message);
        check(true, message, LogSeverity::Error);
    }

    SECTION("LOG_ERROR_IF false") {
        auto message = "ERROR_IF false!";
        HIGHFIVE_LOG_ERROR_IF(false, message);
        check(false, message, LogSeverity::Error);
    }
}

#define HIGHFIVE_STRINGIFY_VALUE(s) HIGHFIVE_STRINGIFY_NAME(s)
#define HIGHFIVE_STRINGIFY_NAME(s)  #s


TEST_CASE("Version Numbers") {
    int major = HIGHFIVE_VERSION_MAJOR;
    int minor = HIGHFIVE_VERSION_MINOR;
    int patch = HIGHFIVE_VERSION_PATCH;
    std::string version = HIGHFIVE_STRINGIFY_VALUE(HIGHFIVE_VERSION);

    auto expected = std::to_string(major) + "." + std::to_string(minor) + "." +
                    std::to_string(patch);

    CHECK(version == expected);
    CHECK(HIGHFIVE_VERSION_STRING == expected);
}

#undef HIGHFIVE_STRINGIFY_VALUE
#undef HIGHFIVE_STRINGIFY_NAME
