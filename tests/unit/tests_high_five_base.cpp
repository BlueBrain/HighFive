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

#define BOOST_TEST_MAIN HighFiveTestBase
#include <boost/test/unit_test.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;

BOOST_AUTO_TEST_CASE(HighFiveBasic) {
    const std::string FILE_NAME("h5tutr_dset.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    BOOST_CHECK_EQUAL(file.getName(), FILE_NAME);

    // Create the data space for the dataset.
    std::vector<size_t> dims{4, 6};

    DataSpace dataspace(dims);

    // check if the dataset exist
    bool dataset_exist = file.exist(DATASET_NAME + "_double");
    BOOST_CHECK(!dataset_exist);

    // Create a dataset with double precision floating points
    DataSet dataset_double = file.createDataSet(DATASET_NAME + "_double", dataspace,
                                                AtomicType<double>());

    BOOST_CHECK_EQUAL(file.getObjectName(0), DATASET_NAME + "_double");

    {
        // check if it exist again
        dataset_exist = file.exist(DATASET_NAME + "_double");
        BOOST_CHECK_EQUAL(dataset_exist, true);

        // and also try to recreate it to the sake of exception testing
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(
            {
                DataSet fail_duplicated = file.createDataSet(
                    DATASET_NAME + "_double", dataspace, AtomicType<double>());
            },
            DataSetException);
    }

    DataSet dataset_size_t = file.createDataSet<size_t>(DATASET_NAME + "_size_t",
                                                        dataspace);
}

BOOST_AUTO_TEST_CASE(HighFiveSilence) {
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
    BOOST_CHECK_EQUAL(buffer[0], '\0');

    // restore the dyn allocated buffer
    // or using stderr will segfault when buffer get out of scope
    fflush(stderr);
    setvbuf(stderr, NULL, _IONBF, 0);
}

BOOST_AUTO_TEST_CASE(HighFiveOpenMode) {
    const std::string FILE_NAME("openmodes.h5");
    const std::string DATASET_NAME("dset");

    std::remove(FILE_NAME.c_str());

    SilenceHDF5 silencer;

    // Attempt open file only ReadWrite should fail (wont create)
    BOOST_CHECK_THROW({ File file(FILE_NAME, File::ReadWrite); }, FileException);

    // But with Create flag should be fine
    { File file(FILE_NAME, File::ReadWrite | File::Create); }

    // But if its there and exclusive is given, should fail
    BOOST_CHECK_THROW({ File file(FILE_NAME, File::ReadWrite | File::Excl); },
                      FileException);
    // ReadWrite and Excl flags are fine together (posix)
    std::remove(FILE_NAME.c_str());
    { File file(FILE_NAME, File::ReadWrite | File::Excl); }
    // All three are fine as well (as long as the file does not exist)
    std::remove(FILE_NAME.c_str());
    { File file(FILE_NAME, File::ReadWrite | File::Create | File::Excl); }

    // Just a few combinations are incompatible, detected by hdf5lib
    BOOST_CHECK_THROW({ File file(FILE_NAME, File::Truncate | File::Excl); },
                      FileException);

    std::remove(FILE_NAME.c_str());
    BOOST_CHECK_THROW({ File file(FILE_NAME, File::Truncate | File::Excl); },
                      FileException);

    // But in most cases we will truncate and that should always work
    { File file(FILE_NAME, File::Truncate); }
    std::remove(FILE_NAME.c_str());
    { File file(FILE_NAME, File::Truncate); }

    // Last but not least, defaults should be ok
    { File file(FILE_NAME); }     // ReadOnly
    { File file(FILE_NAME, 0); }  // force empty-flags, does open without flags
}


BOOST_AUTO_TEST_CASE(HighFiveGroupAndDataSetDefaultCtr) {
    const std::string FILE_NAME("h5_group_test.h5");
    const std::string DATASET_NAME("dset");
    File file(FILE_NAME, File::Truncate);
    auto ds = file.createDataSet(DATASET_NAME, std::vector<int>{1, 2, 3, 4, 5});

    DataSet d2;  // deprecated as it constructs unsafe objects
    // d2.getFile();  // runtime error
    BOOST_CHECK_EQUAL(d2.isValid(), false);
    d2 = ds;  // copy
    BOOST_CHECK_EQUAL(d2.isValid(), true);
}


BOOST_AUTO_TEST_CASE(HighFiveGroupAndDataSet) {
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

        DataSet dataset_absolute = file.createDataSet(
            GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" + DATASET_NAME, dataspace,
            AtomicType<double>());

        DataSet dataset_relative = nested.createDataSet(DATASET_NAME, dataspace,
                                                        AtomicType<double>());

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
            BOOST_CHECK_THROW(file.createDataSet(CHUNKED_DATASET_NAME, dataspace,
                                                 AtomicType<double>(), badChunking0),
                              DataSetException);

            BOOST_CHECK_THROW(file.createDataSet(CHUNKED_DATASET_NAME, dataspace,
                                                 AtomicType<double>(), badChunking1),
                              DataSetException);
        }

        // here we use the other signature
        DataSet dataset_chunked = file.createDataSet<float>(
            CHUNKED_DATASET_NAME, dataspace, goodChunking, cacheConfig);

        // Here we resize to smaller than the chunking size
        DataSet dataset_chunked_small = file.createDataSet<float>(
            CHUNKED_DATASET_SMALL_NAME, dataspace, goodChunking);

        dataset_chunked_small.resize({1, 1});
    }
    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);
        Group g1 = file.getGroup(GROUP_NAME1);
        Group g2 = file.getGroup(GROUP_NAME2);
        Group nested_group2 = g2.getGroup(GROUP_NESTED_NAME);

        DataSet dataset_absolute = file.getDataSet(GROUP_NAME1 + "/" + GROUP_NESTED_NAME +
                                                   "/" + DATASET_NAME);
        BOOST_CHECK_EQUAL(4, dataset_absolute.getSpace().getDimensions()[0]);

        DataSet dataset_relative = nested_group2.getDataSet(DATASET_NAME);
        BOOST_CHECK_EQUAL(4, dataset_relative.getSpace().getDimensions()[0]);

        DataSetAccessProps accessProps;
        accessProps.add(Caching(13, 1024, 0.5));
        DataSet dataset_chunked = file.getDataSet(CHUNKED_DATASET_NAME, accessProps);
        BOOST_CHECK_EQUAL(4, dataset_chunked.getSpace().getDimensions()[0]);

        DataSet dataset_chunked_small = file.getDataSet(CHUNKED_DATASET_SMALL_NAME);
        BOOST_CHECK_EQUAL(1, dataset_chunked_small.getSpace().getDimensions()[0]);
    }
}

BOOST_AUTO_TEST_CASE(HighFiveExtensibleDataSet) {
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
        DataSet dataset = file.createDataSet(DATASET_NAME, dataspace,
                                             AtomicType<long double>(), props);

        // Write into the initial part of the dataset
        dataset.select({0, 0}, {3, 1}).write(t1);

        // Resize the dataset to a larger size
        dataset.resize({4, 6});

        BOOST_CHECK_EQUAL(4, dataset.getSpace().getDimensions()[0]);
        BOOST_CHECK_EQUAL(6, dataset.getSpace().getDimensions()[1]);

        // Write into the new part of the dataset
        dataset.select({3, 3}, {1, 3}).write(t2);

        SilenceHDF5 silencer;
        // Try resize out of bounds
        BOOST_CHECK_THROW(dataset.resize({18, 1}), DataSetException);
        // Try resize invalid dimensions
        BOOST_CHECK_THROW(dataset.resize({1, 2, 3}), DataSetException);
    }

    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);

        DataSet dataset_absolute = file.getDataSet("/" + DATASET_NAME);
        const auto dims = dataset_absolute.getSpace().getDimensions();
        long double values[4][6];
        dataset_absolute.read(values);
        BOOST_CHECK_EQUAL(4, dims[0]);
        BOOST_CHECK_EQUAL(6, dims[1]);

        BOOST_CHECK_EQUAL(t1[0][0], values[0][0]);
        BOOST_CHECK_EQUAL(t1[1][0], values[1][0]);
        BOOST_CHECK_EQUAL(t1[2][0], values[2][0]);

        BOOST_CHECK_EQUAL(t2[0][0], values[3][3]);
        BOOST_CHECK_EQUAL(t2[0][1], values[3][4]);
        BOOST_CHECK_EQUAL(t2[0][2], values[3][5]);
    }
}

BOOST_AUTO_TEST_CASE(HighFiveRefCountMove) {
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

        DataSet d1 = file.createDataSet(GROUP_NAME1 + DATASET_NAME, dataspace,
                                        AtomicType<double>());

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
                    BOOST_CHECK_EQUAL(v, 1);
                } else {
                    BOOST_CHECK_EQUAL(v, 0);
                }
            }
        }

        // force copy
        Group g2 = *g_ptr;

        // add a subgroup
        g2.createGroup("blabla");
    }
}

BOOST_AUTO_TEST_CASE(HighFiveSimpleListing) {
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
        BOOST_CHECK_EQUAL(2, n_elem);

        std::vector<std::string> elems = file.listObjectNames();
        BOOST_CHECK_EQUAL(2, elems.size());
        std::vector<std::string> reference_elems;
        for (int i = 0; i < 2; ++i) {
            std::ostringstream ss;
            ss << GROUP_NAME_CORE << "_" << i;
            reference_elems.push_back(ss.str());
        }

        BOOST_CHECK_EQUAL_COLLECTIONS(elems.begin(), elems.end(), reference_elems.begin(),
                                      reference_elems.end());
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
        BOOST_CHECK_EQUAL(50, n_elem);

        std::vector<std::string> elems = g_nest.listObjectNames();
        BOOST_CHECK_EQUAL(50, elems.size());
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

        BOOST_CHECK_EQUAL_COLLECTIONS(elems.begin(), elems.end(), reference_elems.begin(),
                                      reference_elems.end());
    }
}

BOOST_AUTO_TEST_CASE(DataTypeEqualSimple) {
    AtomicType<double> d_var;
    AtomicType<size_t> size_var;
    AtomicType<double> d_var_test;
    AtomicType<size_t> size_var_cpy(size_var);
    AtomicType<int> int_var;
    AtomicType<unsigned> uint_var;

    // check different type matching
    BOOST_CHECK(d_var == d_var_test);
    BOOST_CHECK(d_var != size_var);

    // check type copy matching
    BOOST_CHECK(size_var_cpy == size_var);

    // check sign change not matching
    BOOST_CHECK(int_var != uint_var);
}

BOOST_AUTO_TEST_CASE(DataTypeEqualTakeBack) {
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

    BOOST_CHECK(s == dataset.getDataType());
    BOOST_CHECK(d != dataset.getDataType());

    // Test getAddress and expect deprecation warning
    auto addr = dataset.getInfo().getAddress();
    BOOST_CHECK(addr != 0);
}

BOOST_AUTO_TEST_CASE(DataSpaceTest) {
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
    BOOST_CHECK_NE(space.getId(), space2.getId());

    // verify space id are consistent
    BOOST_CHECK_EQUAL(space.getDimensions().size(), 2);
    BOOST_CHECK_EQUAL(space.getDimensions()[0], 10);
    BOOST_CHECK_EQUAL(space.getDimensions()[1], 1);
}

BOOST_AUTO_TEST_CASE(DataSpaceVectorTest) {
    // Create 1D shortcut dataspace
    DataSpace space(7);

    BOOST_CHECK_EQUAL(space.getDimensions().size(), 1);
    BOOST_CHECK_EQUAL(space.getDimensions()[0], 7);

    // Initializer list (explicit)
    DataSpace space3({8, 9, 10});
    auto space3_res = space3.getDimensions();
    std::vector<size_t> space3_ans{8, 9, 10};

    BOOST_CHECK_EQUAL_COLLECTIONS(space3_res.begin(), space3_res.end(),
                                  space3_ans.begin(), space3_ans.end());

    // Verify 2D works (note that without the {}, this matches the iterator
    // constructor)
    DataSpace space2(std::vector<size_t>{3, 4});

    auto space2_res = space2.getDimensions();
    std::vector<size_t> space2_ans{3, 4};

    BOOST_CHECK_EQUAL_COLLECTIONS(space2_res.begin(), space2_res.end(),
                                  space2_ans.begin(), space2_ans.end());
}

BOOST_AUTO_TEST_CASE(DataSpaceVariadicTest) {
    // Create 1D shortcut dataspace
    DataSpace space1{7};

    auto space1_res = space1.getDimensions();
    std::vector<size_t> space1_ans{7};

    BOOST_CHECK_EQUAL_COLLECTIONS(space1_res.begin(), space1_res.end(),
                                  space1_ans.begin(), space1_ans.end());

    // Initializer list (explicit)
    DataSpace space3{8, 9, 10};

    auto space3_res = space3.getDimensions();
    std::vector<size_t> space3_ans{8, 9, 10};

    BOOST_CHECK_EQUAL_COLLECTIONS(space3_res.begin(), space3_res.end(),
                                  space3_ans.begin(), space3_ans.end());

    // Verify 2D works using explicit syntax
    DataSpace space2{3, 4};

    auto space2_res = space2.getDimensions();
    std::vector<size_t> space2_ans{3, 4};

    BOOST_CHECK_EQUAL_COLLECTIONS(space2_res.begin(), space2_res.end(),
                                  space2_ans.begin(), space2_ans.end());

    // Verify 2D works using old syntax (this used to match the iterator!)
    DataSpace space2b(3, 4);

    auto space2b_res = space2b.getDimensions();
    std::vector<size_t> space2b_ans{3, 4};

    BOOST_CHECK_EQUAL_COLLECTIONS(space2b_res.begin(), space2b_res.end(),
                                  space2b_ans.begin(), space2b_ans.end());
}

BOOST_AUTO_TEST_CASE(ChunkingConstructorsTest) {
    Chunking first(1, 2, 3);

    auto first_res = first.getDimensions();
    std::vector<hsize_t> first_ans{1, 2, 3};

    BOOST_CHECK_EQUAL_COLLECTIONS(first_res.begin(), first_res.end(), first_ans.begin(),
                                  first_ans.end());

    Chunking second{1, 2, 3};

    auto second_res = second.getDimensions();
    std::vector<hsize_t> second_ans{1, 2, 3};

    BOOST_CHECK_EQUAL_COLLECTIONS(second_res.begin(), second_res.end(),
                                  second_ans.begin(), second_ans.end());

    Chunking third({1, 2, 3});

    auto third_res = third.getDimensions();
    std::vector<hsize_t> third_ans{1, 2, 3};

    BOOST_CHECK_EQUAL_COLLECTIONS(third_res.begin(), third_res.end(), third_ans.begin(),
                                  third_ans.end());
}

BOOST_AUTO_TEST_CASE(HighFiveReadWriteShortcut) {
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

    std::vector<int> result;
    dataset.read(result);
    BOOST_CHECK_EQUAL_COLLECTIONS(vec.begin(), vec.end(), result.begin(), result.end());

    std::string read_in;
    dataset.getAttribute("str").read(read_in);
    BOOST_CHECK_EQUAL(read_in, at_contents);

    int out_int = 0;
    ds_int.read(out_int);
    BOOST_CHECK_EQUAL(my_int, out_int);

    decltype(my_nested) out_nested;
    ds_nested.read(out_nested);

    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            BOOST_CHECK_EQUAL(my_nested[i][j], out_nested[i][j]);
        }
    }

    // Plain c arrays. 1D
    {
        int int_c_array[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        DataSet ds_int2 = file.createDataSet("/TmpCArrayInt", int_c_array);

        decltype(int_c_array) int_c_array_out;
        ds_int2.read(int_c_array_out);
        for (size_t i = 0; i < 10; ++i) {
            BOOST_CHECK_EQUAL(int_c_array[i], int_c_array_out[i]);
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
                BOOST_CHECK_EQUAL(char_c_2darray[i][j], char_c_2darray_out[i][j]);
            }
        }
    }
}

template <typename T>
void readWriteAttributeVectorTest() {
    std::ostringstream filename;
    filename << "h5_rw_attribute_vec_" << typeNameHelper<T>() << "_test.h5";

    std::srand((unsigned)std::time(0));
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
        BOOST_CHECK_EQUAL(n, 0);

        std::vector<std::string> all_attribute_names = g.listAttributeNames();
        BOOST_CHECK_EQUAL(all_attribute_names.size(), 0);

        bool has_attribute = g.hasAttribute("my_attribute");
        BOOST_CHECK_EQUAL(has_attribute, false);

        Attribute a1 = g.createAttribute<T>("my_attribute", DataSpace::From(vec));
        a1.write(vec);

        // check now that we effectively have an attribute listable
        std::size_t n2 = g.getNumberAttributes();
        BOOST_CHECK_EQUAL(n2, 1);

        has_attribute = g.hasAttribute("my_attribute");
        BOOST_CHECK(has_attribute);

        all_attribute_names = g.listAttributeNames();
        BOOST_CHECK_EQUAL(all_attribute_names.size(), 1);
        BOOST_CHECK_EQUAL(all_attribute_names[0], std::string("my_attribute"));

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

        BOOST_CHECK_EQUAL(vec.size(), x_size);
        BOOST_CHECK_EQUAL(result1.size(), x_size);

        for (size_t i = 0; i < x_size; ++i)
            BOOST_CHECK_EQUAL(result1[i], vec[i]);

        Attribute a2_read = file.getDataSet("/dummy_group/dummy_dataset")
                                .getAttribute("my_attribute_copy");
        a2_read.read(result2);

        BOOST_CHECK_EQUAL(vec.size(), x_size);
        BOOST_CHECK_EQUAL(result2.size(), x_size);

        for (size_t i = 0; i < x_size; ++i)
            BOOST_CHECK_EQUAL(result2[i], vec[i]);

        std::vector<int> v;  // with const would print a nice err msg
        file.getDataSet("/dummy_group/dummy_dataset")
            .getAttribute("version_test")
            .read(v);
    }

    // Delete some attributes
    {
        // From group
        auto g = file.getGroup("dummy_group");
        g.deleteAttribute("my_attribute");
        auto n = g.getNumberAttributes();
        BOOST_CHECK_EQUAL(n, 0);

        // From dataset
        auto d = file.getDataSet("/dummy_group/dummy_dataset");
        d.deleteAttribute("my_attribute_copy");
        n = g.getNumberAttributes();
        BOOST_CHECK_EQUAL(n, 0);
    }
}

BOOST_AUTO_TEST_CASE(ReadWriteAttributeVectorString) {
    readWriteAttributeVectorTest<std::string>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReadWriteAttributeVector, T, dataset_test_types) {
    readWriteAttributeVectorTest<T>();
}

BOOST_AUTO_TEST_CASE(datasetOffset) {
    std::string filename = "datasetOffset.h5";
    std::string dsetname = "dset";
    const size_t size_dataset = 20;

    File file(filename, File::ReadWrite | File::Create | File::Truncate);
    std::vector<int> data(size_dataset);
    DataSet ds = file.createDataSet<int>(dsetname, DataSpace::From(data));
    ds.write(data);
    DataSet ds_read = file.getDataSet(dsetname);
    BOOST_CHECK(ds_read.getOffset() > 0);
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

        BOOST_CHECK_EQUAL(slice.getSpace().getDimensions()[0], size_x);
        BOOST_CHECK_EQUAL(slice.getMemSpace().getDimensions()[0], count_x);

        slice.read(result);

        BOOST_CHECK_EQUAL(result.size(), 5);

        for (size_t i = 0; i < count_x; ++i) {
            BOOST_CHECK_EQUAL(values[i + offset_x], result[i]);
        }
    }

    // select cherry pick
    {
        // read it back
        Vector result;
        std::vector<size_t> ids{1, 3, 4, 7};

        Selection slice = dataset.select(ElementSet(ids));

        BOOST_CHECK_EQUAL(slice.getSpace().getDimensions()[0], size_x);
        BOOST_CHECK_EQUAL(slice.getMemSpace().getDimensions()[0], ids.size());

        slice.read(result);

        BOOST_CHECK_EQUAL(result.size(), ids.size());

        for (size_t i = 0; i < ids.size(); ++i) {
            const std::size_t id = ids[i];
            BOOST_CHECK_EQUAL(values[id], result[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(selectionArraySimpleString) {
    selectionArraySimpleTest<std::string>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(selectionArraySimple, T, dataset_test_types) {
    selectionArraySimpleTest<T>();
}

BOOST_AUTO_TEST_CASE(selectionByElementMultiDim) {
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
        BOOST_CHECK_EQUAL(value, 5);
    }

    {
        int value[2];
        set.select(ElementSet{0, 0, 2, 2}).read(value);
        BOOST_CHECK_EQUAL(value[0], 1);
        BOOST_CHECK_EQUAL(value[1], 9);
    }

    {
        int value[2];
        set.select(ElementSet{{0, 1}, {1, 2}}).read(value);
        BOOST_CHECK_EQUAL(value[0], 2);
        BOOST_CHECK_EQUAL(value[1], 6);
    }

    {
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(set.select(ElementSet{0, 1, 2}), DataSpaceException);
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

    BOOST_CHECK_EQUAL(slice.getSpace().getDimensions()[0], x_size);
    BOOST_CHECK_EQUAL(slice.getMemSpace().getDimensions()[0], x_size);

    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < x_size; ++j)
            BOOST_CHECK_EQUAL(result[j][i], values[j][columns[i]]);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(columnSelection, T, numerical_test_types) {
    columnSelectionTest<T>();
}

template <typename T>
void attribute_scalar_rw() {
    std::ostringstream filename;
    filename << "h5_rw_attribute_scalar_rw" << typeNameHelper<T>() << "_test.h5";

    File h5file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    ContentGenerate<T> generator;

    const T attribute_value(generator());

    Group g = h5file.createGroup("metadata");

    bool family_exist = g.hasAttribute("family");
    BOOST_CHECK(!family_exist);

    // write a scalar attribute
    {
        T out(attribute_value);
        Attribute att = g.createAttribute<T>("family", DataSpace::From(out));
        att.write(out);
    }

    h5file.flush();

    // test if attribute exist
    family_exist = g.hasAttribute("family");
    BOOST_CHECK(family_exist);

    // read back a scalar attribute
    {
        T res;
        Attribute att = g.getAttribute("family");
        att.read(res);
        BOOST_CHECK_EQUAL(res, attribute_value);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(attribute_scalar_rw_all, T, dataset_test_types) {
    attribute_scalar_rw<T>();
}

BOOST_AUTO_TEST_CASE(attribute_scalar_rw_string) {
    attribute_scalar_rw<std::string>();
}

// regression test https://github.com/BlueBrain/HighFive/issues/98
BOOST_AUTO_TEST_CASE(HighFiveOutofDimension) {
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
        BOOST_CHECK_EQUAL(ndim, 0);

        auto dims = recovered_d1.getDimensions();
        BOOST_CHECK_EQUAL(dims.size(), 0);
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
                BOOST_CHECK_EQUAL(result[i][j], array[i][j]);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReadWriteShuffleDeflate, T, numerical_test_types) {
    readWriteShuffleDeflateTest<T>();
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
                BOOST_CHECK_EQUAL(result[i][j], array[i][j]);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReadWriteSzip, T, dataset_test_types) {
    // SZIP is not consistently available across distributions.
    if (H5Zfilter_avail(H5Z_FILTER_SZIP)) {
        readWriteSzipTest<T>();
    } else {
        BOOST_CHECK_THROW(readWriteSzipTest<T>(), PropertyException);
    }
}

// Broadcasting is supported
BOOST_AUTO_TEST_CASE(ReadInBroadcastDims) {

    const std::string FILE_NAME("h5_missmatch1_dset.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims_a{1, 3};
    std::vector<size_t> dims_b{3, 1};

    // 1D input / output vectors
    std::vector<double> some_data{5.0, 6.0, 7.0};
    std::vector<double> data_a;
    std::vector<double> data_b;

    DataSpace dataspace_a(dims_a);
    DataSpace dataspace_b(dims_b);

    // Create a dataset with double precision floating points
    DataSet dataset_a = file.createDataSet(DATASET_NAME + "_a", dataspace_a,
                                           AtomicType<double>());
    DataSet dataset_b = file.createDataSet(DATASET_NAME + "_b", dataspace_b,
                                           AtomicType<double>());

    dataset_a.write(some_data);
    dataset_b.write(some_data);

    DataSet out_a = file.getDataSet(DATASET_NAME + "_a");
    DataSet out_b = file.getDataSet(DATASET_NAME + "_b");

    out_a.read(data_a);
    out_b.read(data_b);

    BOOST_CHECK_EQUAL_COLLECTIONS(data_a.begin(), data_a.end(), some_data.begin(),
                                  some_data.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(data_b.begin(), data_b.end(), some_data.begin(),
                                  some_data.end());
}

BOOST_AUTO_TEST_CASE(HighFiveRecursiveGroups) {
    const std::string FILE_NAME("h5_ds_exist.h5");
    const std::string GROUP_1("group1"), GROUP_2("group2");
    const std::string DS_PATH = GROUP_1 + "/" + GROUP_2;
    const std::string DS_NAME = "ds";

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    BOOST_CHECK_EQUAL(file.getName(), FILE_NAME);

    // Without parents creating both groups will fail
    {
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(file.createGroup(DS_PATH, false), std::exception);
    }
    Group g2 = file.createGroup(DS_PATH);

    std::vector<double> some_data{5.0, 6.0, 7.0};
    g2.createDataSet(DS_NAME, some_data);

    BOOST_CHECK(file.exist(GROUP_1));

    Group g1 = file.getGroup(GROUP_1);
    BOOST_CHECK(g1.exist(GROUP_2));

    // checks with full path
    BOOST_CHECK(file.exist(DS_PATH));
    BOOST_CHECK(file.exist(DS_PATH + "/" + DS_NAME));

    // Check with wrong middle path (before would raise Exception)
    BOOST_CHECK_EQUAL(file.exist(std::string("blabla/group2")), false);

    // Using root slash
    BOOST_CHECK(file.exist(std::string("/") + DS_PATH));

    // Check unlink with existing group
    BOOST_CHECK(g1.exist(GROUP_2));
    g1.unlink(GROUP_2);
    BOOST_CHECK(!g1.exist(GROUP_2));

    // Check unlink with non-existing group
    {
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(g1.unlink("x"), HighFive::GroupException);
    }
}

BOOST_AUTO_TEST_CASE(HighFiveInspect) {
    const std::string FILE_NAME("group_info.h5");
    const std::string GROUP_1("group1");
    const std::string DS_NAME = "ds";

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    Group g = file.createGroup(GROUP_1);

    std::vector<double> some_data{5.0, 6.0, 7.0};
    g.createDataSet(DS_NAME, some_data);

    BOOST_CHECK(file.getLinkType(GROUP_1) == LinkType::Hard);

    {
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(file.getLinkType("x"), HighFive::GroupException);
    }

    BOOST_CHECK(file.getObjectType(GROUP_1) == ObjectType::Group);
    BOOST_CHECK(file.getObjectType(GROUP_1 + "/" + DS_NAME) == ObjectType::Dataset);
    BOOST_CHECK(g.getObjectType(DS_NAME) == ObjectType::Dataset);

    {
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(file.getObjectType(DS_NAME), HighFive::GroupException);
    }

    // Data type
    auto ds = g.getDataSet(DS_NAME);
    auto dt = ds.getDataType();
    BOOST_CHECK(dt.getClass() == DataTypeClass::Float);
    BOOST_CHECK(dt.getSize() == 8);
    BOOST_CHECK(dt.string() == "Float64");

    // meta
    BOOST_CHECK(ds.getType() == ObjectType::Dataset);  // internal
    BOOST_CHECK(ds.getInfo().getRefCount() == 1);
}


BOOST_AUTO_TEST_CASE(HighFiveGetPath) {

    File file("getpath.h5", File::ReadWrite | File::Create | File::Truncate);

    int number = 100;
    Group group = file.createGroup("group");
    DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
    dataset.write(number);
    std::string string_list("Very important DataSet!");
    Attribute attribute = dataset.createAttribute<std::string>("attribute", DataSpace::From(string_list));
    attribute.write(string_list);

    BOOST_CHECK_EQUAL("/", file.getPath());
    BOOST_CHECK_EQUAL("/group", group.getPath());
    BOOST_CHECK_EQUAL("/group/data", dataset.getPath());
    BOOST_CHECK_EQUAL("attribute", attribute.getName());
    BOOST_CHECK_EQUAL("/group/data", attribute.getPath());

    BOOST_CHECK(file == dataset.getFile());
    BOOST_CHECK(file == attribute.getFile());

    // Destroy file early (it should live inside Dataset/Group)
    std::unique_ptr<File> f2(new File("getpath.h5"));
    const auto& d2 = f2->getDataSet("/group/data");
    f2.reset(nullptr);
    BOOST_CHECK_EQUAL(d2.getFile().getPath(), "/");

}

BOOST_AUTO_TEST_CASE(HighFiveSoftLinks) {
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
        BOOST_CHECK(data == data_out);
    }

    {
        const std::string EXTERNAL_LINK_PATH("/external_link/to_ds");
        File file2("link_external_to.h5", File::ReadWrite | File::Create | File::Truncate);
        file2.createExternalLink(EXTERNAL_LINK_PATH, FILE_NAME, DS_PATH);

        std::vector<int> data_out;
        file2.getDataSet(EXTERNAL_LINK_PATH).read(data_out);
        BOOST_CHECK(data == data_out);
    }
}

BOOST_AUTO_TEST_CASE(HighFiveRename) {

    File file("move.h5", File::ReadWrite | File::Create | File::Truncate);

    int number = 100;

    {
        Group group = file.createGroup("group");
        DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
        dataset.write(number);
        std::string path = dataset.getPath();
        BOOST_CHECK_EQUAL("/group/data", path);
    }

    file.rename("/group/data", "/new/group/new/data");

    {
        DataSet dataset = file.getDataSet("/new/group/new/data");
        std::string path = dataset.getPath();
        BOOST_CHECK_EQUAL("/new/group/new/data", path);
        int read;
        dataset.read(read);
        BOOST_CHECK_EQUAL(number, read);
    }
}

BOOST_AUTO_TEST_CASE(HighFiveRenameRelative) {

    File file("move.h5", File::ReadWrite | File::Create | File::Truncate);
    Group group = file.createGroup("group");

    int number = 100;

    {
        DataSet dataset = group.createDataSet("data", DataSpace(1), AtomicType<int>());
        dataset.write(number);
        BOOST_CHECK_EQUAL("/group/data", dataset.getPath());
    }

    group.rename("data", "new_data");

    {
        DataSet dataset = group.getDataSet("new_data");
        BOOST_CHECK_EQUAL("/group/new_data", dataset.getPath());
        int read;
        dataset.read(read);
        BOOST_CHECK_EQUAL(number, read);
    }
}

BOOST_AUTO_TEST_CASE(HighFivePropertyObjects) {
    const auto& plist1 = FileCreateProps::Default();  // get const-ref, otherwise copies
    BOOST_CHECK_EQUAL(plist1.getId(), H5P_DEFAULT);
    BOOST_CHECK_EQUAL(plist1.isValid(), false);       // not valid -> no inc_ref
    auto plist2 = plist1;  // copy  (from Object)
    BOOST_CHECK_EQUAL(plist2.getId(), H5P_DEFAULT);

    // Underlying object is same (singleton holder of H5P_DEFAULT)
    const auto& other_plist_type = LinkCreateProps::Default();
    BOOST_CHECK_EQUAL((void*)&plist1, (void*)&other_plist_type);

    LinkCreateProps plist_g;
    BOOST_CHECK_EQUAL(plist_g.getId(), H5P_DEFAULT);
    BOOST_CHECK_EQUAL(plist_g.isValid(), false);

    plist_g.add(CreateIntermediateGroup());
    BOOST_CHECK(plist_g.isValid());
    auto plist_g2 = plist_g;
    BOOST_CHECK(plist_g2.isValid());
}


typedef struct {
    int m1;
    int m2;
    int m3;
} CSL1;

typedef struct {
    CSL1 csl1;
} CSL2;


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

BOOST_AUTO_TEST_CASE(HighFiveCompounds) {
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

        BOOST_CHECK_EQUAL(result.size(), 2);
        BOOST_CHECK_EQUAL(result[0].m1, 1);
        BOOST_CHECK_EQUAL(result[0].m2, 1);
        BOOST_CHECK_EQUAL(result[0].m3, 1);
        BOOST_CHECK_EQUAL(result[1].m1, 2);
        BOOST_CHECK_EQUAL(result[1].m2, 3);
        BOOST_CHECK_EQUAL(result[1].m3, 4);
    }

    {  // Nested
        auto dataset = file.createDataSet(DATASET_NAME2, DataSpace(2), t2);

        std::vector<CSL2> csl = {{{1, 1, 1}, {2, 3, 4}}};
        dataset.write(csl);

        file.flush();
        std::vector<CSL2> result = {{{1, 1, 1}, {2, 3, 4}}};
        dataset.select({0}, {2}).read(result);

        BOOST_CHECK_EQUAL(result.size(), 2);
        BOOST_CHECK_EQUAL(result[0].csl1.m1, 1);
        BOOST_CHECK_EQUAL(result[0].csl1.m2, 1);
        BOOST_CHECK_EQUAL(result[0].csl1.m3, 1);
        BOOST_CHECK_EQUAL(result[1].csl1.m1, 2);
        BOOST_CHECK_EQUAL(result[1].csl1.m2, 3);
        BOOST_CHECK_EQUAL(result[1].csl1.m3, 4);
    }

    // Test the constructor from hid
    CompoundType t1_from_hid(t1);
    BOOST_ASSERT(t1 == t1_from_hid);

    CompoundType t2_from_hid(t2);
    BOOST_ASSERT(t2 == t2_from_hid);
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
                     {"gcm3", t2,                   }});
    return t1;
}

CompoundType create_compound_Child() {
    auto nestedType = create_compound_GrandChild();
    return CompoundType{ { { "grandChild", nestedType,           },
                           { "cm1",   AtomicType<uint32_t>{}} } };
}

CompoundType create_compound_Parent() {
    auto nestedType = create_compound_Child();
    return CompoundType{ { { "pm1",   AtomicType<uint32_t>{}},
                           { "child", nestedType,           }} };
}

HIGHFIVE_REGISTER_TYPE(GrandChild, create_compound_GrandChild)
HIGHFIVE_REGISTER_TYPE(Child,      create_compound_Child)
HIGHFIVE_REGISTER_TYPE(Parent,     create_compound_Parent)

BOOST_AUTO_TEST_CASE(HighFiveCompoundsNested) {
    const std::string FILE_NAME("nested_compounds_test.h5");
    const std::string DATASET_NAME("/a");

    {  // Write
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
        auto type = create_compound_Parent();

        auto dataset = file.createDataSet(DATASET_NAME, DataSpace(2), type);
        BOOST_CHECK_EQUAL(dataset.getDataType().getSize(), 20);

        std::vector<Parent> csl = { Parent{ 1, Child{ GrandChild{1,1,1}, 1 } },
                                    Parent{ 2, Child{ GrandChild{3,4,5}, 6 } } };
        dataset.write(csl);
    }

    {  // Read
        File file(FILE_NAME, File::ReadOnly);
        std::vector<Parent> result;
        auto dataset = file.getDataSet(DATASET_NAME);
        BOOST_CHECK_EQUAL(dataset.getDataType().getSize(), 20);
        dataset.select({0}, {2}).read(result);

        BOOST_CHECK_EQUAL(result.size(), 2);
        BOOST_CHECK_EQUAL(result[0].pm1, 1);
        BOOST_CHECK_EQUAL(result[0].child.grandChild.gcm1, 1);
        BOOST_CHECK_EQUAL(result[0].child.grandChild.gcm2, 1);
        BOOST_CHECK_EQUAL(result[0].child.grandChild.gcm3, 1);
        BOOST_CHECK_EQUAL(result[0].child.cm1, 1);
        BOOST_CHECK_EQUAL(result[1].pm1, 2);
        BOOST_CHECK_EQUAL(result[1].child.grandChild.gcm1, 3);
        BOOST_CHECK_EQUAL(result[1].child.grandChild.gcm2, 4);
        BOOST_CHECK_EQUAL(result[1].child.grandChild.gcm3, 5);
        BOOST_CHECK_EQUAL(result[1].child.cm1, 6);
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

BOOST_AUTO_TEST_CASE(HighFiveEnum) {
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

        BOOST_CHECK_EQUAL(result, Position::FIRST);
    }

    {  // Scoped enum
        auto e1 = create_enum_direction();
        e1.commit(file, "Direction");

        auto dataset = file.createDataSet(DATASET_NAME2, DataSpace(5), e1);
        std::vector<Direction> robot_moves({Direction::BACKWARD, Direction::FORWARD,
                                            Direction::FORWARD, Direction::LEFT,
                                            Direction::LEFT});
        dataset.write(robot_moves);

        file.flush();

        std::vector<Direction> result;
        dataset.read(result);

        BOOST_CHECK_EQUAL(result[0], Direction::BACKWARD);
        BOOST_CHECK_EQUAL(result[1], Direction::FORWARD);
        BOOST_CHECK_EQUAL(result[2], Direction::FORWARD);
        BOOST_CHECK_EQUAL(result[3], Direction::LEFT);
        BOOST_CHECK_EQUAL(result[4], Direction::LEFT);
    }
}

BOOST_AUTO_TEST_CASE(HighFiveFixedString) {
    const std::string FILE_NAME("array_atomic_types.h5");
    const std::string GROUP_1("group1");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    char raw_strings[][10] = {"abcd", "1234"};

    /// This will not compile - only char arrays - hits static_assert with a nice error
    // file.createDataSet<int[10]>(DS_NAME, DataSpace(2)));

    {  // But char should be fine
        auto ds = file.createDataSet<char[10]>("ds1", DataSpace(2));
        BOOST_CHECK(ds.getDataType().getClass() == DataTypeClass::String);
        ds.write(raw_strings);
    }

    {  // char[] is, by default, int8
        auto ds2 = file.createDataSet("ds2", raw_strings);
        BOOST_CHECK(ds2.getDataType().getClass() == DataTypeClass::Integer);
    }

    {  // String Truncate happens low-level if well setup
        auto ds3 = file.createDataSet<char[6]>(
            "ds3", DataSpace::FromCharArrayStrings(raw_strings));
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
        BOOST_CHECK_THROW(file.createDataSet<char[10]>("ds5", DataSpace(2)).write(buffer),
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
        BOOST_CHECK(array_back.size() == 3);
        BOOST_CHECK(array_back[0] == std::string("000"));
        BOOST_CHECK(array_back[1] == std::string("111"));
        BOOST_CHECK(array_back[2] == std::string("222"));
        BOOST_CHECK(array_back.getString(1) == "111");
        BOOST_CHECK(array_back.front() == std::string("000"));
        BOOST_CHECK(array_back.back() == std::string("222"));
        BOOST_CHECK(array_back.data() == std::string("000"));
        array_back.data()[0] = 'x';
        BOOST_CHECK(array_back.data() == std::string("x00"));

        for (auto& raw_elem : array_back) {
            raw_elem[1] = 'y';
        }
        BOOST_CHECK(array_back.getString(1) == "1y1");
        for (auto iter = array_back.cbegin(); iter != array_back.cend(); ++iter) {
            BOOST_CHECK((*iter)[1] == 'y');
        }
    }
}

BOOST_AUTO_TEST_CASE(HighFiveFixedLenStringArrayStructure) {

    using fixed_array_t = FixedLenStringArray<10>;
    // increment the characters of a string written in a std::array
    auto increment_string = [](const fixed_array_t::value_type arr) {
        fixed_array_t::value_type output(arr);
        for (auto& c : output) {
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
        BOOST_CHECK_EQUAL(arr2.size(), 4);
    }

    // manipulate FixedLenStringArray with std::transform
    {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::transform(arr1.begin(), arr1.end(), std::back_inserter(arr),
                           increment_string);
        }
        BOOST_CHECK_EQUAL(arr.size(), 2);
        BOOST_CHECK_EQUAL(arr[0], std::string("1111111"));
        BOOST_CHECK_EQUAL(arr[1], std::string("2222222"));
    }

    // manipulate FixedLenStringArray with std::transform and reverse iterator
    {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::copy(arr1.rbegin(), arr1.rend(), std::back_inserter(arr));
        }
        BOOST_CHECK_EQUAL(arr.size(), 2);
        BOOST_CHECK_EQUAL(arr[0], std::string("1111111"));
        BOOST_CHECK_EQUAL(arr[1], std::string("0000000"));
    }

    // manipulate FixedLenStringArray with std::remove_copy_if
    {
        fixed_array_t arr2;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::remove_copy_if(arr1.begin(), arr1.end(), std::back_inserter(arr2),
                                [](const fixed_array_t::value_type& s) {
                                    return std::strncmp(s.data(), "1111111", 7) == 0;
                                });
        }
        BOOST_CHECK_EQUAL(arr2.size(), 1);
        BOOST_CHECK_EQUAL(arr2[0], std::string("0000000"));
    }
}


BOOST_AUTO_TEST_CASE(HighFiveFixedLenStringArrayAttribute) {
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
        BOOST_CHECK_EQUAL(arr.size(), 2);
        BOOST_CHECK_EQUAL(arr[0], std::string("Hello"));
        BOOST_CHECK_EQUAL(arr[1], std::string("world"));
    }

}


BOOST_AUTO_TEST_CASE(HighFiveReference) {
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
        BOOST_CHECK_EQUAL(2, refdataset.getSpace().getDimensions()[0]);
        auto refs = std::vector<Reference>();
        refdataset.read(refs);
        BOOST_CHECK_THROW(refs[0].dereference<Group>(file), HighFive::ReferenceException);
        auto data_ds = refs[0].dereference<DataSet>(file);
        std::vector<double> rdata;
        data_ds.read(rdata);
        for (size_t i = 0; i < rdata.size(); ++i) {
            BOOST_CHECK_EQUAL(rdata[i], vec1[i]);
        }

        auto group = refs[1].dereference<Group>(file);
        DataSet data_ds2 = group.getDataSet(DATASET2_NAME);
        std::vector<double> rdata2;
        data_ds2.read(rdata2);
        for (size_t i = 0; i < rdata2.size(); ++i) {
            BOOST_CHECK_EQUAL(rdata2[i], vec2[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(HighFiveReadWriteConsts) {
    const std::string FILE_NAME("3d_dataset_from_flat.h5");
    const std::string DATASET_NAME("dset");
    const std::array<std::size_t, 3> DIMS{3, 3, 3};
    using datatype = int;

    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    DataSpace dataspace = DataSpace(DIMS);

    DataSet dataset = file.createDataSet<datatype>(DATASET_NAME, dataspace);
    std::vector<datatype> const t1(DIMS[0] * DIMS[1] * DIMS[2], 1);
    auto raw_3d_vec_const = reinterpret_cast<datatype const *const *const *>(t1.data());
    dataset.write(raw_3d_vec_const);

    std::vector<std::vector<std::vector<datatype>>> result;
    dataset.read(result);
    for (const auto& vec2d : result) {
        for (const auto& vec1d : vec2d) {
            BOOST_CHECK(vec1d == (std::vector<datatype>{1, 1, 1}));
        }
    }

}

#ifdef H5_USE_EIGEN

template <typename T>
void test_eigen_vec(File& file,
                    const std::string& test_flavor,
                    const T& vec_input,
                    T& vec_output) {
    const std::string DS_NAME = "ds";
    file.createDataSet(DS_NAME + test_flavor, vec_input).write(vec_input);
    file.getDataSet(DS_NAME + test_flavor).read(vec_output);
    BOOST_CHECK(vec_input == vec_output);
}

BOOST_AUTO_TEST_CASE(HighFiveEigen) {
    const std::string FILE_NAME("test_eigen.h5");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);
    std::string DS_NAME_FLAVOR;

    // std::vector<of vector <of POD>>
    {
        DS_NAME_FLAVOR = "VectorOfVectorOfPOD";
        std::vector<std::vector<float>> vec_in{
            {5.0f, 6.0f, 7.0f}, {5.1f, 6.1f, 7.1f}, {5.2f, 6.2f, 7.2f}};
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

    // std::vector<of EigenMatrixXd> - exception
    {
        const std::string DS_NAME = "ds";
        DS_NAME_FLAVOR = "VectorEigenMatrixXdExc";

        Eigen::MatrixXd m1 = 100. * Eigen::MatrixXd::Random(20, 5);
        Eigen::MatrixXd m2 = 100. * Eigen::MatrixXd::Random(20, 5);
        std::vector<Eigen::MatrixXd> vec_in;
        vec_in.push_back(m1);
        vec_in.push_back(m2);
        file.createDataSet(DS_NAME + DS_NAME_FLAVOR, vec_in).write(vec_in);

        std::vector<Eigen::MatrixXd> vec_out_exception;
        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(
            file.getDataSet(DS_NAME + DS_NAME_FLAVOR).read(vec_out_exception),
            HighFive::DataSetException);
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

    // boost::mulit_array<of EigenMatrixXd> - exception
    {
        const std::string DS_NAME = "ds";
        DS_NAME_FLAVOR = "BMultiEigenMatrixXdExc";

        boost::multi_array<Eigen::MatrixXd, 3> vec_in(boost::extents[3][2][2]);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec_in[i][j][k] = Eigen::MatrixXd::Random(3, 3);
                }
            }
        }

        file.createDataSet(DS_NAME + DS_NAME_FLAVOR, vec_in).write(vec_in);
        boost::multi_array<Eigen::MatrixXd, 3> vec_out_exception(boost::extents[3][2][2]);

        SilenceHDF5 silencer;
        BOOST_CHECK_THROW(
            file.getDataSet(DS_NAME + DS_NAME_FLAVOR).read(vec_out_exception),
            HighFive::DataSetException);
    }

#endif
}
#endif
