/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <H5Ipublic.h>
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
    {
        File file(file_name, File::ReadWrite | File::Create);
    }

    // But if its there and exclusive is given, should fail
    CHECK_THROWS_AS(File(file_name, File::ReadWrite | File::Excl), FileException);
    // ReadWrite and Excl flags are fine together (posix)
    std::remove(file_name.c_str());
    {
        File file(file_name, File::ReadWrite | File::Excl);
    }
    // All three are fine as well (as long as the file does not exist)
    std::remove(file_name.c_str());
    {
        File file(file_name, File::ReadWrite | File::Create | File::Excl);
    }

    // Just a few combinations are incompatible, detected by hdf5lib
    CHECK_THROWS_AS(File(file_name, File::Truncate | File::Excl), FileException);

    std::remove(file_name.c_str());
    CHECK_THROWS_AS(File(file_name, File::Truncate | File::Excl), FileException);

    // But in most cases we will truncate and that should always work
    {
        File file(file_name, File::Truncate);
    }
    std::remove(file_name.c_str());
    {
        File file(file_name, File::Truncate);
    }

    // Last but not least, defaults should be ok
    {
        File file(file_name);
    }  // ReadOnly
}

void check_access_mode(File::AccessMode mode) {
    CHECK(any(mode));
    CHECK(((File::ReadOnly | mode) & File::ReadOnly) == File::AccessMode::ReadOnly);
    CHECK(!any(File::ReadOnly & mode));
    CHECK(((File::ReadOnly | mode) ^ mode) == File::AccessMode::ReadOnly);
    CHECK((File::ReadOnly & ~mode) == File::AccessMode::ReadOnly);
    CHECK(!any(mode & ~mode));
}

TEST_CASE("File::AccessMode") {
    CHECK(!any(File::AccessMode::None));
    CHECK(any(File::ReadOnly));

    check_access_mode(File::ReadWrite);
    check_access_mode(File::Truncate);
    check_access_mode(File::Excl);
    check_access_mode(File::Debug);
    check_access_mode(File::Create);
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

template <class T>
void check_invalid_hid_Object(T& obj) {
    auto silence = SilenceHDF5();

    CHECK(!obj.isValid());
    CHECK(obj.getId() == H5I_INVALID_HID);

    CHECK_THROWS(obj.getInfo());
    CHECK_THROWS(obj.getType());
}

template <class T, class U>
void check_invalid_hid_NodeTraits(T& obj, const U& linkable) {
    auto silence = SilenceHDF5();

    auto data_space = DataSpace{2, 3};
    auto data_type = HighFive::create_datatype<double>();
    auto data = std::vector<double>{1.0, 2.0, 3.0};
    auto gcpl = GroupCreateProps();

    CHECK_THROWS(obj.createDataSet("foo", data_space, data_type));
    CHECK_THROWS(obj.template createDataSet<double>("foo", data_space));
    CHECK_THROWS(obj.createDataSet("foo", data));

    CHECK_THROWS(obj.getDataSet("foo"));
    CHECK_THROWS(obj.createGroup("foo"));
    CHECK_THROWS(obj.createGroup("foo", gcpl));
    CHECK_THROWS(obj.getGroup("foo"));
    CHECK_THROWS(obj.getDataType("foo"));
    CHECK_THROWS(obj.getNumberObjects());
    CHECK_THROWS(obj.getObjectName(0));
    CHECK_THROWS(obj.rename("foo", "bar"));
    CHECK_THROWS(obj.listObjectNames());
    CHECK_THROWS(obj.exist("foo"));
    CHECK_THROWS(obj.unlink("foo"));
    CHECK_THROWS(obj.getLinkType("foo"));
    CHECK_THROWS(obj.getObjectType("foo"));
    CHECK_THROWS(obj.createSoftLink("foo", linkable));
    CHECK_THROWS(obj.createSoftLink("foo", "bar"));
    CHECK_THROWS(obj.createExternalLink("foo", "bar", "baz"));
    CHECK_THROWS(obj.createHardLink("foo", linkable));
}

template <class T>
void check_invalid_hid_DataSet(T& obj) {
    auto silence = SilenceHDF5();

    CHECK_THROWS(obj.getStorageSize());
    CHECK_THROWS(obj.getOffset());
    CHECK_THROWS(obj.getMemSpace());
    CHECK_THROWS(obj.resize({1, 2, 3}));
    CHECK_THROWS(obj.getDimensions());
    CHECK_THROWS(obj.getElementCount());
    CHECK_THROWS(obj.getCreatePropertyList());
    CHECK_THROWS(obj.getAccessPropertyList());
}

template <class T>
void check_invalid_hid_SliceTraits(T& obj) {
    auto silence = SilenceHDF5();

    auto slab = HighFive::HyperSlab(RegularHyperSlab({0}));
    auto space = DataSpace{3};
    auto set = ElementSet{0, 1, 3};
    auto data = std::vector<double>{1.0, 2.0, 3.0};
    auto type = create_datatype<double>();
    auto cols = std::vector<size_t>{0, 2, 3};

    CHECK_THROWS(obj.select(slab));
    CHECK_THROWS(obj.select(slab, space));
    CHECK_THROWS(obj.select({0}, {3}));
    CHECK_THROWS(obj.select(cols));
    CHECK_THROWS(obj.select(set));

    CHECK_THROWS(obj.template read<double>());
    CHECK_THROWS(obj.read(data));
    CHECK_THROWS(obj.read_raw(data.data(), type));
    CHECK_THROWS(obj.template read_raw<double>(data.data()));

    CHECK_THROWS(obj.write(data));
    CHECK_THROWS(obj.write_raw(data.data(), type));
    CHECK_THROWS(obj.template write_raw<double>(data.data()));
}

template <class T>
void check_invalid_hid_PathTraits(T& obj) {
    auto silence = SilenceHDF5();

    CHECK_THROWS(obj.getPath());
    CHECK_THROWS(obj.getFile());
}

template <class T>
void check_invalid_hid_AnnotateTraits(T& obj) {
    auto silence = SilenceHDF5();

    auto space = DataSpace{3};
    auto data = std::vector<double>{1.0, 2.0, 3.0};
    auto type = create_datatype<double>();

    CHECK_THROWS(obj.createAttribute("foo", space, type));
    CHECK_THROWS(obj.template createAttribute<double>("foo", space));
    CHECK_THROWS(obj.createAttribute("foo", data));

    CHECK_THROWS(obj.deleteAttribute("foo"));
    CHECK_THROWS(obj.getAttribute("foo"));
    CHECK_THROWS(obj.getNumberAttributes());
    CHECK_THROWS(obj.listAttributeNames());
    CHECK_THROWS(obj.hasAttribute("foo"));
}

template <class T>
void check_invalid_hid_Group(T& obj) {
    auto silence = SilenceHDF5();

    CHECK_THROWS(obj.getEstimatedLinkInfo());
    CHECK_THROWS(obj.getCreatePropertyList());
}

TEST_CASE("Test default DataSet constructor") {
    DataSet ds;
    check_invalid_hid_Object(ds);
    check_invalid_hid_DataSet(ds);
    check_invalid_hid_SliceTraits(ds);
    check_invalid_hid_AnnotateTraits(ds);
    check_invalid_hid_PathTraits(ds);

    File file("h5_default_dset_ctor.h5", File::Truncate);
    ds = file.createDataSet("dset", std::vector<int>{1, 2, 3, 4, 5});
    CHECK(ds.isValid());
}

TEST_CASE("Test default Group constructor") {
    File file("h5_default_group_ctor.h5", File::Truncate);
    Group linkable = file.createGroup("bar");

    Group grp;
    check_invalid_hid_Object(grp);
    check_invalid_hid_NodeTraits(grp, linkable);
    check_invalid_hid_AnnotateTraits(grp);
    check_invalid_hid_PathTraits(grp);

    grp = file.createGroup("grp");

    CHECK(grp.isValid());
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

    SECTION("null initializer_list") {
        auto space = DataSpace{DataSpace::dataspace_null};
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

    SECTION("scalar initializer_list") {
        auto space = DataSpace{DataSpace::dataspace_scalar};
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
    for (unsigned i = 0; i < x_size; i++) {
        vec[i] = i * 2;
    }
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
    SECTION("int-c-array") {
        int int_c_array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        DataSet ds_int2 = file.createDataSet("/TmpCArrayInt", int_c_array);

        decltype(int_c_array) int_c_array_out;
        ds_int2.read(int_c_array_out);
        for (size_t i = 0; i < 10; ++i) {
            REQUIRE(int_c_array[i] == int_c_array_out[i]);
        }
    }

    // Plain c arrays. 2D
    SECTION("char-c-array") {
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


// Ensure that "all" combinations of `ProductSet` are being instantiated.
class CompileProductSet {
    using Point = size_t;
    using Points = std::vector<size_t>;
    using Slice = std::array<size_t, 2>;
    using Slices = std::vector<Slice>;

    void all() {
        one_arg();
        two_args();
        three_args();
    }

    template <class... Preamble>
    void zero_args() {
        ProductSet(Preamble()...);
    }

    template <class... Preamble>
    void one_arg() {
        zero_args<Preamble..., Point>();
        zero_args<Preamble..., Points>();
        zero_args<Preamble..., Slice>();
        zero_args<Preamble..., Slices>();
    }

    template <class... Preamble>
    void two_args() {
        one_arg<Preamble..., Point>();
        one_arg<Preamble..., Points>();
        one_arg<Preamble..., Slice>();
        one_arg<Preamble..., Slices>();
    }

    template <class... Preamble>
    void three_args() {
        two_args<Preamble..., Point>();
        two_args<Preamble..., Points>();
        two_args<Preamble..., Slice>();
        two_args<Preamble..., Slices>();
    }
};

template <class S, class Y, class X>
void check_product_set_shape(const S& subarray, const Y& yslices, const X& xslices) {
    std::vector<size_t> subshape{0, 0};

    for (auto yslice: yslices) {
        subshape[0] += yslice[1] - yslice[0];
    }

    for (auto xslice: xslices) {
        subshape[1] += xslice[1] - xslice[0];
    }

    REQUIRE(subarray.size() == subshape[0]);
    for (const auto& v: subarray) {
        REQUIRE(v.size() == subshape[1]);
    }
}

template <class S, class Y, class X>
void check_product_set_values(const S& array,
                              const S& subarray,
                              const Y& yslices,
                              const X& xslices) {
    size_t il = 0;
    for (const auto& yslice: yslices) {
        for (size_t ig = yslice[0]; ig < yslice[1]; ++ig) {
            size_t jl = 0;

            for (const auto& xslice: xslices) {
                for (size_t jg = xslice[0]; jg < xslice[1]; ++jg) {
                    REQUIRE(subarray[il][jl] == array[ig][jg]);
                    ++jl;
                }
            }
            ++il;
        }
    }
}

template <class S, class Y, class X>
void check(const S& array, const S& subarray, const Y& yslices, const X& xslices) {
    check_product_set_shape(subarray, yslices, xslices);
    check_product_set_values(array, subarray, yslices, xslices);
}


TEST_CASE("productSet") {
    using Slice = std::array<size_t, 2>;
    using Slices = std::vector<Slice>;
    using Point = size_t;
    using Points = std::vector<size_t>;

    const std::string file_name("h5_test_product_set.h5");

    auto generate = [](size_t n, size_t m, auto f) {
        auto x = std::vector<std::vector<double>>(n);
        for (size_t i = 0; i < n; ++i) {
            x[i] = std::vector<double>(m);
        }

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < m; ++j) {
                x[i][j] = f(i, j);
            }
        }

        return x;
    };

    auto array = generate(6, 12, [](size_t i, size_t j) { return double(i) + double(j) * 0.01; });

    auto file = File(file_name, File::Truncate);
    auto dset = file.createDataSet("dset", array);

    SECTION("rR") {
        std::vector<std::vector<double>> subarray;

        auto yslice = Slice{1, 3};
        auto yslices = Slices{yslice};
        auto xslices = Slices{{0, 1}, {3, 5}};

        dset.select(ProductSet(yslice, xslices)).read(subarray);

        check(array, subarray, yslices, xslices);
    }

    SECTION("Rr") {
        std::vector<std::vector<double>> subarray;

        auto yslices = Slices{{0, 1}, {3, 5}};
        auto xslice = Slice{1, 3};
        auto xslices = Slices{xslice};

        dset.select(ProductSet(yslices, xslice)).read(subarray);

        check(array, subarray, yslices, xslices);
    }

    SECTION("RP") {
        std::vector<std::vector<double>> subarray;

        auto yslices = Slices{{0, 1}, {3, 5}};
        auto xpoints = Points{2, 4, 5};
        auto xslices = Slices{{2, 3}, {4, 6}};

        dset.select(ProductSet(yslices, xpoints)).read(subarray);

        check(array, subarray, yslices, xslices);
    }

    SECTION("pR") {
        std::vector<std::vector<double>> subarray;

        auto ypoint = Point{2};
        auto yslices = Slices{{2, 3}};
        auto xslices = Slices{{0, 1}, {3, 5}};

        dset.select(ProductSet(ypoint, xslices)).read(subarray);

        check(array, subarray, yslices, xslices);
    }

    SECTION("pp") {
        std::vector<std::vector<double>> subarray;

        auto xpoint = Point{3};
        auto ypoint = Point{2};
        auto yslices = Slices{{2, 3}};
        auto xslices = Slices{{3, 4}};

        dset.select(ProductSet(ypoint, xpoint)).read(subarray);
        check(array, subarray, yslices, xslices);
    }

    SECTION("PP") {
        std::vector<std::vector<double>> subarray;

        auto xpoints = Points{0, 3, 4};
        auto ypoints = Points{2, 3};
        auto yslices = Slices{{2, 4}};
        auto xslices = Slices{{0, 1}, {3, 5}};

        dset.select(ProductSet(ypoints, xpoints)).read(subarray);
        check(array, subarray, yslices, xslices);
    }

    SECTION("RR") {
        std::vector<std::vector<double>> subarray;

        auto yslices = Slices{{2, 4}};
        auto xslices = Slices{{0, 1}, {3, 5}};

        dset.select(ProductSet(yslices, xslices)).read(subarray);
        check(array, subarray, yslices, xslices);
    }
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

template <class CreateTraits>
void check_broadcast_scalar_memspace(File& file,
                                     const std::string& name,
                                     const std::vector<size_t>& dims) {
    auto datatype = create_datatype<double>();
    auto obj = CreateTraits::create(file, name, DataSpace(dims), datatype);

    double expected = 3.0;
    obj.write(expected);

    auto actual = obj.template read<double>();
    CHECK(actual == expected);
}

TEST_CASE("Broadcast scalar memspace, dset") {
    File file("h5_broadcast_scalar_memspace_dset.h5", File::Truncate);

    SECTION("[1]") {
        check_broadcast_scalar_memspace<testing::DataSetCreateTraits>(file, "dset", {1});
    }

    SECTION("[1, 1, 1]") {
        check_broadcast_scalar_memspace<testing::DataSetCreateTraits>(file, "dset", {1, 1, 1});
    }
}

TEST_CASE("Broadcast scalar memspace, attr") {
    File file("h5_broadcast_scalar_memspace_attr.h5", File::Truncate);

    SECTION("[1]") {
        check_broadcast_scalar_memspace<testing::AttributeCreateTraits>(file, "attr", {1});
    }

    SECTION("[1, 1, 1]") {
        check_broadcast_scalar_memspace<testing::AttributeCreateTraits>(file, "attr", {1, 1, 1});
    }
}

template <class CreateTraits>
void check_broadcast_scalar_filespace(File& file, const std::string& name) {
    auto datatype = create_datatype<double>();
    auto obj = CreateTraits::create(file, name, DataSpace::Scalar(), datatype);

    auto value = std::vector<double>{3.0};

    REQUIRE_THROWS(obj.write(value));
    REQUIRE_THROWS(obj.template read<std::vector<double>>());
    REQUIRE_THROWS(obj.read(value));
}

TEST_CASE("Broadcast scalar filespace, dset") {
    File file("h5_broadcast_scalar_filespace_dset.h5", File::Truncate);
    check_broadcast_scalar_filespace<testing::DataSetCreateTraits>(file, "dset");
}

TEST_CASE("Broadcast scalar filespace, attr") {
    File file("h5_broadcast_scalar_filespace_attr.h5", File::Truncate);
    check_broadcast_scalar_filespace<testing::AttributeCreateTraits>(file, "attr");
}

TEST_CASE("squeeze") {
    CHECK(detail::squeeze({}, {}) == std::vector<size_t>{});
    CHECK(detail::squeeze({3, 1, 1}, {}) == std::vector<size_t>{3, 1, 1});
    CHECK(detail::squeeze({3, 1, 1}, {2, 1}) == std::vector<size_t>{3});
    CHECK(detail::squeeze({1, 3, 1, 2}, {2, 0}) == std::vector<size_t>{3, 2});

    CHECK_THROWS(detail::squeeze({3, 1, 1}, {3}));
    CHECK_THROWS(detail::squeeze({3, 1, 1}, {0}));
    CHECK_THROWS(detail::squeeze({}, {0}));
}

template <class CreateTraits>
void check_modify_memspace(File& file, const std::string& name) {
    auto expected_values = std::vector<double>{1.0, 2.0, 3.0};
    auto values = std::vector<std::vector<double>>{expected_values};

    auto obj = CreateTraits::create(file, name, values);
    SECTION("squeeze") {
        auto actual_values = obj.squeezeMemSpace({0}).template read<std::vector<double>>();

        REQUIRE(actual_values.size() == expected_values.size());
        for (size_t i = 0; i < actual_values.size(); ++i) {
            REQUIRE(actual_values[i] == expected_values[i]);
        }
    }

    SECTION("reshape") {
        auto actual_values = obj.reshapeMemSpace({3}).template read<std::vector<double>>();

        REQUIRE(actual_values.size() == expected_values.size());
        for (size_t i = 0; i < actual_values.size(); ++i) {
            REQUIRE(actual_values[i] == expected_values[i]);
        }
    }
}

TEST_CASE("Modify MemSpace, dset") {
    File file("h5_modify_memspace_dset.h5", File::Truncate);
    check_modify_memspace<testing::DataSetCreateTraits>(file, "dset");
}

TEST_CASE("Modify MemSpace, attr") {
    File file("h5_modify_memspace_attr.h5", File::Truncate);
    check_modify_memspace<testing::AttributeCreateTraits>(file, "attr");
}

template <class CreateTraits>
void check_modify_scalar_filespace(File& file, const std::string& name) {
    auto expected_value = 3.0;

    auto obj = CreateTraits::create(file, name, expected_value);
    SECTION("reshape") {
        auto actual_values = obj.reshapeMemSpace({1}).template read<std::vector<double>>();

        REQUIRE(actual_values.size() == 1);
        REQUIRE(actual_values[0] == expected_value);
    }
}

TEST_CASE("Modify Scalar FileSpace, dset") {
    File file("h5_modify_scalar_filespace_dset.h5", File::Truncate);
    check_modify_scalar_filespace<testing::DataSetCreateTraits>(file, "dset");
}

TEST_CASE("Modify Scalar FileSpace, attr") {
    File file("h5_modify_scalar_filespace_attr.h5", File::Truncate);
    check_modify_scalar_filespace<testing::AttributeCreateTraits>(file, "attr");
}

template <class CreateTraits>
void check_modify_scalar_memspace(File& file, const std::string& name) {
    auto expected_value = std::vector<double>{3.0};

    auto obj = CreateTraits::create(file, name, expected_value);
    SECTION("squeeze") {
        auto actual_value = obj.squeezeMemSpace({0}).template read<double>();
        REQUIRE(actual_value == expected_value[0]);
    }

    SECTION("reshape") {
        auto actual_value = obj.reshapeMemSpace({}).template read<double>();
        REQUIRE(actual_value == expected_value[0]);
    }
}

TEST_CASE("Modify Scalar MemSpace, dset") {
    File file("h5_modify_scalar_memspace_dset.h5", File::Truncate);
    check_modify_scalar_memspace<testing::DataSetCreateTraits>(file, "dset");
}

TEST_CASE("Modify Scalar MemSpace, attr") {
    File file("h5_modify_scalar_memspace_attr.h5", File::Truncate);
    check_modify_scalar_memspace<testing::AttributeCreateTraits>(file, "attr");
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
        attr.read_raw(actual);

        for (size_t i = 0; i < n; ++i) {
            REQUIRE(expected[i] == actual[i]);
        }
    }

    SECTION("WriteReadCycleDataSet") {
        auto dset = file.createAttribute("dset", dataspace, datatype);
        dset.write_raw(expected);
        dset.read_raw(actual);

        for (size_t i = 0; i < n; ++i) {
            REQUIRE(expected[i] == actual[i]);
        }
    }

    delete[] expected;
    delete[] actual;
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

#ifdef HIGHFIVE_TEST_EIGEN

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

        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
    }

    // Eigen MatrixXd
    {
        ds_name_flavor = "EigenMatrixXd";
        Eigen::MatrixXd vec_in = 100. * Eigen::MatrixXd::Random(20, 5);
        Eigen::MatrixXd vec_out(20, 5);

        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
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

        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
    }

#ifdef HIGHFIVE_TEST_BOOST
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

        test_eigen_vec(file, ds_name_flavor, vec_in, vec_out);
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

#if defined(HIGHFIVE_VERSION_PRERELEASE)
    int prerelease = HIGHFIVE_VERSION_PRERELEASE;
    expected += "-beta" + std::to_string(prerelease);
#endif

    CHECK(HIGHFIVE_VERSION_STRING == expected);
}

#undef HIGHFIVE_STRINGIFY_VALUE
#undef HIGHFIVE_STRINGIFY_NAME
