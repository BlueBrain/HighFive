/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

#ifdef HIGHFIVE_CPP11_ENABLE
#include <memory>
#endif

#ifdef USE_BLOSC

#include <H5Ppublic.h>
#include <highfive/H5Filter.hpp>
#include "blosc_filter.h"

#endif


#include <cstdio>

#include <highfive/H5File.hpp>
#include <highfive/H5Utility.hpp>
#include <highfive/EigenUtils.hpp>

#define BOOST_TEST_MODULE HighFiveTest

#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#ifdef H5_USE_BOOST
#include <boost/multi_array.hpp>
#endif

using namespace HighFive;
typedef boost::mpl::list<float, double> floating_numerics_test_types;
typedef boost::mpl::list<int, unsigned int, long, unsigned long, unsigned char,
                         char, float, double, long long, unsigned long long>
    numerical_test_types;
typedef boost::mpl::list<int, unsigned int, long, unsigned long, unsigned char,
                         char, float, double, std::string>
    dataset_test_types;

template <typename T, typename Func>
void generate2D(T* table, size_t x, size_t y, Func& func) {
    for (size_t i = 0; i < x; i++) {
        for (size_t j = 0; j < y; j++) {
            table[i][j] = func();
        }
    }
}

template <typename T, typename Func>
void generate2D(std::vector<std::vector<T> >& vec, size_t x, size_t y,
                Func& func) {
    vec.resize(x);
    for (size_t i = 0; i < x; i++) {
        vec[i].resize(y);
        for (size_t j = 0; j < y; j++) {
            vec[i][j] = func();
        }
    }
}

template <typename T>
struct ContentGenerate {
    ContentGenerate(T init_val = T(0), T inc_val = T(1) + T(1) / T(10))
        : _init(init_val), _inc(inc_val) {}

    T operator()() {
        T ret = _init;
        _init += _inc;
        return ret;
    }

    T _init, _inc;
};

template <>
struct ContentGenerate<char> {
    ContentGenerate() : _init('a') {}

    char operator()() {
        char ret = _init;
        if (++_init >= ('a' + 26))
            _init = 'a';
        return ret;
    }

    char _init;
};

template <>
struct ContentGenerate<std::string> {
    ContentGenerate() {}

    std::string operator()() {
        ContentGenerate<char> gen;
        std::string random_string;
        const int size_string = std::rand() % 254;
        random_string.resize(size_string);
        std::generate(random_string.begin(), random_string.end(), gen);
        return random_string;
    }
};

BOOST_AUTO_TEST_CASE(HighFiveBasic) {

    const std::string FILE_NAME("h5tutr_dset.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    BOOST_CHECK_EQUAL(file.getName(), FILE_NAME);

    // Create the data space for the dataset.
    std::vector<size_t> dims;
    dims.push_back(4);
    dims.push_back(6);

    DataSpace dataspace(dims);

    // check if the dataset exist
    bool dataset_exist = file.exist(DATASET_NAME + "_double");
    BOOST_CHECK_EQUAL(dataset_exist, false);

    // Create a dataset with double precision floating points
    DataSet dataset_double = file.createDataSet(
        DATASET_NAME + "_double", dataspace, AtomicType<double>());

    BOOST_CHECK_EQUAL(file.getObjectName(0), DATASET_NAME + "_double");

    {
        // check if it exist again
        dataset_exist = file.exist(DATASET_NAME + "_double");
        BOOST_CHECK_EQUAL(dataset_exist, true);

        // and also try to recreate it to the sake of exception testing
        BOOST_CHECK_THROW(
            {
                DataSet fail_duplicated = file.createDataSet(
                    DATASET_NAME + "_double", dataspace, AtomicType<double>());
            },
            DataSetException);
    }

    DataSet dataset_size_t =
        file.createDataSet<size_t>(DATASET_NAME + "_size_t", dataspace);
}

BOOST_AUTO_TEST_CASE(HighFiveGroups) {

    const std::string FILE_NAME("h5nest_grp.h5");

    std::vector<std::string> group_names = {"1", "2", "3"};
    // Create a new file using the default property lists.
    {
        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        BOOST_CHECK_EQUAL(file.getName(), FILE_NAME);


        Group group = file.createGroups(group_names);
    }

    File file(FILE_NAME, File::ReadOnly);
    bool group_exist = file.exist("1");
    BOOST_CHECK_EQUAL(group_exist, true);
    group_exist = file.exist("2");
    BOOST_CHECK_EQUAL(group_exist, false);
    group_exist = file.getGroup("1").exist("2");
    BOOST_CHECK_EQUAL(group_exist, true);
    group_exist = file.getGroup("1").getGroup("2").exist("3");
    BOOST_CHECK_EQUAL(group_exist, true);
}






BOOST_AUTO_TEST_CASE(HighFiveSilence) {

    // Setting up a buffer for stderr so we can detect if the stack trace
    // was disabled
    fflush(stderr);
    char buffer[1024];
    memset(buffer, 0, sizeof(char) * 1024);
    setvbuf(stderr, buffer, _IOLBF, 1023);

    try
    {
        SilenceHDF5 silence;
        File file("nonexistent", File::ReadOnly);
    }
    catch (const FileException&)
    {}
    BOOST_CHECK_EQUAL(buffer[0], '\0');
    
    // restore the dyn allocated buffer
    // or using stderr will segfault when buffer get out of scope
    fflush(stderr);
    setvbuf(stderr, nullptr, _IONBF, 0);
}
/*

BOOST_AUTO_TEST_CASE(HighFiveException) {

    // Create a new file
    File file1("random_file_123",
               File::ReadWrite | File::Create | File::Truncate);

    BOOST_CHECK_THROW(
        {
            // triggers a file creation conflict
            File file2("random_file_123", File::ReadWrite | File::Create);
        },
        FileException);
}
*/

BOOST_AUTO_TEST_CASE(HighFiveGroupAndDataSet) {

    const std::string FILE_NAME("h5_group_test.h5");
    const std::string DATASET_NAME("dset");
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
        std::vector<size_t> dims;
        dims.push_back(4);
        dims.push_back(6);

        DataSpace dataspace(dims);

        DataSet dataset_absolute = file.createDataSet(
            GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" + DATASET_NAME,
            dataspace, AtomicType<double>());

        DataSet dataset_relative =
            nested.createDataSet(DATASET_NAME, dataspace, AtomicType<double>());
    }
    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);
        Group g1 = file.getGroup(GROUP_NAME1);
        Group g2 = file.getGroup(GROUP_NAME2);
        Group nested_group2 = g2.getGroup(GROUP_NESTED_NAME);

        DataSet dataset_absolute = file.getDataSet(
            GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" + DATASET_NAME);
        BOOST_CHECK_EQUAL(4, dataset_absolute.getSpace().getDimensions()[0]);

        DataSet dataset_relative = nested_group2.getDataSet(DATASET_NAME);
        BOOST_CHECK_EQUAL(4, dataset_relative.getSpace().getDimensions()[0]);
    }
}

#ifdef HIGHFIVE_CPP11_ENABLE
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
        std::vector<size_t> dims = { 10 , 10 };

        DataSpace dataspace(dims);

        DataSet d1 = file.createDataSet(
            GROUP_NAME1 + DATASET_NAME,
            dataspace, AtomicType<double>());

        double values[10][10] = {{0}};
        values[5][0] = 1;
        d1.write(values);

        // force move
        d1_ptr.reset( new DataSet(std::move(d1)));

        // force copy
        g_ptr.reset(new Group(g1));

    }
    // read it back
    {
        DataSet d2(std::move(*d1_ptr));
        d1_ptr.reset();

        double values[10][10];
        memset(values, 255, 10*10);

        d2.read(values);

        for(std::size_t i =0; i < 10; ++i){
            for(std::size_t j =0;  j < 10; ++j){
                double v = values[i][j];

                if( i == 5 && j == 0){
                    BOOST_CHECK_EQUAL(v, 1);
                }else{
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

#endif


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

        BOOST_CHECK_EQUAL_COLLECTIONS(elems.begin(), elems.end(),
                                      reference_elems.begin(),
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

        BOOST_CHECK_EQUAL_COLLECTIONS(elems.begin(), elems.end(),
                                      reference_elems.begin(),
                                      reference_elems.end());
    }
}

BOOST_AUTO_TEST_CASE(DataTypeEqualSimple) {

    using namespace HighFive;

    AtomicType<double> d_var;

    AtomicType<size_t> size_var;

    AtomicType<double> d_var_test;

    AtomicType<size_t> size_var_cpy(size_var);

    AtomicType<int> int_var;
    AtomicType<unsigned int> uint_var;

    // check different type matching
    BOOST_CHECK(d_var == d_var_test);
    BOOST_CHECK(d_var != size_var);

    // check type copy matching
    BOOST_CHECK(size_var_cpy == size_var);

    // check sign change not matching
    BOOST_CHECK(int_var != uint_var);
}

BOOST_AUTO_TEST_CASE(DataTypeEqualTakeBack) {

    using namespace HighFive;

    const std::string FILE_NAME("h5tutr_dset.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims;
    dims.push_back(10);
    dims.push_back(1);

    DataSpace dataspace(dims);

    // Create a dataset with double precision floating points
    DataSet dataset =
        file.createDataSet<size_t>(DATASET_NAME + "_double", dataspace);

    AtomicType<size_t> s;
    AtomicType<double> d;

    BOOST_CHECK(s == dataset.getDataType());
    BOOST_CHECK(d != dataset.getDataType());
}

BOOST_AUTO_TEST_CASE(DataSpaceTest) {

    using namespace HighFive;

    const std::string FILE_NAME("h5tutr_space.h5");
    const std::string DATASET_NAME("dset");

    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims;
    dims.push_back(10);
    dims.push_back(1);

    DataSpace dataspace(dims);

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

template <typename T>
void readWrite2DArrayTest() {

    std::ostringstream filename;
    filename << "h5_rw_2d_array_" << typeid(T).name() << "_test.h5";

    const std::string DATASET_NAME("dset");

    const size_t x_size = 100;
    const size_t y_size = 10;

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims;
    dims.push_back(x_size);
    dims.push_back(y_size);

    DataSpace dataspace(dims);

    // Create a dataset with arbitrary type
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace);

    T array[x_size][y_size];

    ContentGenerate<T> generator;
    generate2D(array, x_size, y_size, generator);

    dataset.write(array);

    T result[x_size][y_size];

    dataset.read(result);

    for (size_t i = 0; i < x_size; ++i) {
        for (size_t j = 0; j < y_size; ++j) {
            BOOST_CHECK_EQUAL(result[i][j], array[i][j]);
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReadWrite2DArray, T, numerical_test_types) {

    readWrite2DArrayTest<T>();
}

#ifdef USE_BLOSC

template<typename T>
void readWriteVectorCompressionTest() {
    using namespace HighFive;

    std::ostringstream filename;
    filename << "h5_rw_compression_" << typeid(T).name() << "_test.h5";

    std::srand((unsigned int) std::time(0));
    const size_t x_size = 800;
    const std::string DATASET_NAME("dset");
    typename std::vector<T> vec(x_size);

    ContentGenerate<T> generator;
    std::generate(vec.begin(), vec.end(), generator);
    char *version, *date;
    int r;

    /* Register the filter with the library */
    r = register_blosc(&version, &date);
    free(version);
    free(date);
    // Create a new file using the default property lists.
    {
        File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);


        std::vector<size_t> cshape{100};

        Filter filter(cshape, FILTER_BLOSC, r);
        // Create a dataset with double precision floating points
        DataSet dataset = file.createDataSet(DATASET_NAME, DataSpace::From(vec), AtomicType<T>(), filter.getId());

        dataset.write(vec);
    }
    typename std::vector<T> result;
    File file(filename.str(), File::ReadOnly);
    auto dataset = file.getDataSet(DATASET_NAME);
    dataset.read(result);

    BOOST_CHECK_EQUAL(vec.size(), x_size);
    BOOST_CHECK_EQUAL(result.size(), x_size);

    for (size_t i = 0; i < x_size; ++i) {
        // std::cout << result[i] << " " << vec[i] << "  ";
        BOOST_CHECK_EQUAL(result[i], vec[i]);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWriteVectorCompression, T, dataset_test_types) {
    readWriteVectorCompressionTest<T>();
}


#endif


template <typename T>
void readWriteVectorTest() {
    using namespace HighFive;

    std::ostringstream filename;
    filename << "h5_rw_vec_" << typeid(T).name() << "_test.h5";

    std::srand((unsigned int)std::time(0));
    const size_t x_size = 800;
    const std::string DATASET_NAME("dset");
    typename std::vector<T> vec(x_size);
    ContentGenerate<T> generator;
    std::generate(vec.begin(), vec.end(), generator);
    auto filename_string = filename.str();
    // Create a new file using the default property lists.
    File file(filename_string, File::ReadWrite | File::Create | File::Truncate);

    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(vec));

    dataset.write(vec);

    typename std::vector<T> result;

    dataset.read(result);

    BOOST_CHECK_EQUAL(vec.size(), x_size);
    BOOST_CHECK_EQUAL(result.size(), x_size);

    for (size_t i = 0; i < x_size; ++i) {
        // std::cout << result[i] << " " << vec[i] << "  ";
        BOOST_CHECK_EQUAL(result[i], vec[i]);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWriteVector, T, dataset_test_types) {
    readWriteVectorTest<T>();
}


template <typename T>
void readWriteAttributeVectorTest() {
    using namespace HighFive;

    std::ostringstream filename;
    filename << "h5_rw_attribute_vec_" << typeid(T).name() << "_test.h5";

    std::srand((unsigned int)std::time(0));
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

        Attribute a1 =
                g.createAttribute<T>("my_attribute", DataSpace::From(vec));
        a1.write(vec);

        // check now that we effectively have an attribute listable
        std::size_t n2 = g.getNumberAttributes();
        BOOST_CHECK_EQUAL(n2, 1);

        has_attribute = g.hasAttribute("my_attribute");
        BOOST_CHECK_EQUAL(has_attribute, true);

        all_attribute_names = g.listAttributeNames();
        BOOST_CHECK_EQUAL(all_attribute_names.size(), 1);
        BOOST_CHECK_EQUAL(all_attribute_names[0], std::string("my_attribute"));

        // Create the same attribute on a newly created dataset
        DataSet s =
            g.createDataSet("dummy_dataset", DataSpace(1), AtomicType<int>());

        Attribute a2 =
            s.createAttribute<T>("my_attribute_copy", DataSpace::From(vec));
        a2.write(vec);
    }

    typename std::vector<T> result1, result2;

    {
        Attribute a1_read =
            file.getGroup("dummy_group").getAttribute("my_attribute");
        a1_read.read(result1);

        BOOST_CHECK_EQUAL(vec.size(), x_size);
        BOOST_CHECK_EQUAL(result1.size(), x_size);

        for (size_t i = 0; i < x_size; ++i) {
            // std::cout << result[i] << " " << vec[i] << "  ";
            BOOST_CHECK_EQUAL(result1[i], vec[i]);
        }

        Attribute a2_read = file.getDataSet("/dummy_group/dummy_dataset")
                                .getAttribute("my_attribute_copy");
        a2_read.read(result2);

        BOOST_CHECK_EQUAL(vec.size(), x_size);
        BOOST_CHECK_EQUAL(result2.size(), x_size);

        for (size_t i = 0; i < x_size; ++i) {
            // std::cout << result[i] << " " << vec[i] << "  ";
            BOOST_CHECK_EQUAL(result2[i], vec[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReadWriteAttributeVector, T, dataset_test_types) {
    readWriteAttributeVectorTest<T>();
}

template <typename T>
void readWriteVector2DTest() {
    using namespace HighFive;

    std::ostringstream filename;
    filename << "h5_rw_vec_2d_" << typeid(T).name() << "_test.h5";

    const size_t x_size = 10;
    const size_t y_size = 10;
    const std::string DATASET_NAME("dset");
    typename std::vector<std::vector<T> > vec;

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    vec.resize(x_size);
    ContentGenerate<T> generator;

    generate2D(vec, x_size, y_size, generator);

    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(vec));

    dataset.write(vec);

    typename std::vector<std::vector<T> > result;

    dataset.read(result);

    BOOST_CHECK_EQUAL(vec.size(), x_size);
    BOOST_CHECK_EQUAL(result.size(), x_size);

    BOOST_CHECK_EQUAL(vec[0].size(), y_size);
    BOOST_CHECK_EQUAL(result[0].size(), y_size);

    for (size_t i = 0; i < x_size; ++i) {
        for (size_t j = 0; i < y_size; ++i) {
            BOOST_CHECK_EQUAL(result[i][j], vec[i][j]);
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(readWriteVector2D, T, numerical_test_types) {

    readWriteVector2DTest<T>();
}

#ifdef H5_USE_EIGEN
#ifdef USE_BLOSC

template<typename T>
void eigen_matrix_compression_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_eigen_compression_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    char *version, *date;
    auto r = register_blosc(&version, &date);
    free(version);
    free(date);
    std::vector<size_t> cshape{5, 5};

    Filter filter(cshape, FILTER_BLOSC, r);
    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet(DATASET_NAME, DataSpace::From(mat), AtomicType<T>(), filter.getId());

    dataset.write(mat);

    // read it back
    Matrix result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_matrix_compression, T, numerical_test_types) {

    eigen_matrix_compression_Test<T>();
}

#endif



template<typename T>
void eigen_matrix_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.

    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);
    auto tgrp = file.getGroup("/");


    DataSet dataset = tgrp.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));
    dataset.write(mat);

    // read it back
    Matrix result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_matrix, T, numerical_test_types) {

    eigen_matrix_Test<T>();
}


template<typename T>
void R_eigen_matrix_w_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_R_w_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    write_mat_h5(filename.str(), "/", "dset", mat);

    File file(filename.str(), File::ReadOnly);
    auto tgrp = file.getGroup("/");

    Matrix result;
    DataSet dataset = tgrp.getDataSet("dset");
    dataset.read(result);


    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(R_eigen_w_matrix, T, numerical_test_types) {

    R_eigen_matrix_w_Test<T>();
}


template<typename T>
void R_eigen_matrix_w_t_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_R_w_t_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    write_mat_h5(filename.str(), "/", "dset", mat, true);

    Matrix result;

    read_mat_h5(filename.str(), "/", "dset", result);


    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(R_eigen_w_t_matrix, T, numerical_test_types) {

    R_eigen_matrix_w_t_Test<T>();
}


template<typename T>
void R_eigen_matrix_r_t_c_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_R_r_t_c_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.

    write_mat_h5(filename.str(), "/", "dset", mat, true);

    Matrix result;
    Matrix check_res = mat.block(1, 2, 4, 5);

    read_mat_h5(filename.str(), "/", "dset", result, {1, 2}, {4, 5});


    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(check_res(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(R_eigen_r_t_c_matrix, T, numerical_test_types) {

    R_eigen_matrix_r_t_c_Test<T>();
}


template<typename T>
void R_eigen_matrix_w_t_c_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_R_w_t_c_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    Matrix check_res = mat.block(1, 2, 4, 5);
    // Create a new file using the default property lists.
    {
        File file(filename.str(), File::Create | File::ReadWrite);
        auto dataset = file.createDataSet("dset", DataSpace::From(mat, true), AtomicType<T>(), H5P_DEFAULT, true);
        dataset.selectEigen({1, 2}, {4, 5}, {}).write(check_res);

    }
    //write_mat_h5(filename.str(), "/", "dset", mat, true);

    Matrix result;
    //Matrix check_res = mat.block(1, 2, 4, 5);

    read_mat_h5(filename.str(), "/", "dset", result, {1, 2}, {4, 5});


    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 5; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(check_res(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(R_eigen_w_t_c_matrix, T, numerical_test_types) {

    R_eigen_matrix_w_t_c_Test<T>();
}


template<typename T>
void R_eigen_matrix_r_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_R_r_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    {
        File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);
        auto tgrp = file.getGroup("/");


        DataSet dataset = tgrp.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));
        dataset.write(mat);
    }

    // read it back
    Matrix result;
    read_mat_h5<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>(filename.str(), "/", "dset", result);


    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(R_eigen_r_matrix, T, numerical_test_types) {

    R_eigen_matrix_r_Test<T>();
}



template<typename T>
void eigen_matrix_map_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix;
    typedef typename Eigen::Map<Matrix> MapMat;

    std::ostringstream filename;
    filename << "h5_rw_matrix_map_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);


    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));
    dataset.write(mat);

    // read it back
    Matrix result(size_x, size_y);
    MapMat map_res(result.data(), result.rows(), result.cols());

    dataset.read(map_res);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_matrix_map, T, numerical_test_types) {

    eigen_matrix_map_Test<T>();
}


template<typename T>
void eigen_matrix_map_rm_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix_rm;
    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrix_cm;
    typedef typename Eigen::Map<Matrix_rm> MapMat_rm;
    typedef typename Eigen::Map<Matrix_cm> MapMat_cm;

    std::ostringstream filename;
    filename << "h5_rw_matrix_map_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_rm mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);


    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));
    dataset.write(mat);
    Matrix_rm tmat = mat.transpose();

    // read it back
    Matrix_cm result(size_y, size_x);
    MapMat_rm map_res(result.data(), size_x, size_y);

    dataset.read(map_res);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(tmat(j, i), result(j, i));

        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_matrix_map_rm, T, numerical_test_types) {

    eigen_matrix_map_rm_Test<T>();
}



//Ensure that the type conversion performed by the library is equivalent to the one performed by Eigen
template<typename T>
void eigen_fd_matrix_Test() {

    typedef typename std::conditional<std::is_same<T, double>::value, float, double>::type OT;
    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> Matrix_T;
    typedef typename Eigen::Matrix<OT, Eigen::Dynamic, Eigen::Dynamic> Matrix_OT;


    std::ostringstream filename;
    filename << "h5_rw_matrix_eigen_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_T mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }
    Matrix_OT check_mat(mat.template cast<OT>());
    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);


    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));
    dataset.write(mat);

    // read it back
    Matrix_OT result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(check_mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_fd_matrix, T, floating_numerics_test_types) {

    eigen_fd_matrix_Test<T>();
}


template<typename T>
void eigen_cm_rm_matrix_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrix_cm;
    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix_rm;

    std::ostringstream filename;
    filename << "h5_rw_matrix_cm_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_rm mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));

    dataset.write(mat);

    // read it back
    Matrix_cm result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_cm_rm_matrix, T, numerical_test_types) {

    eigen_cm_rm_matrix_Test<T>();
}


template<typename T>
void eigen_rm_cm_matrix_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrix_cm;
    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix_rm;

    std::ostringstream filename;
    filename << "h5_rw_matrix_cm_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 9, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_cm mat(size_x, size_y);

    ContentGenerate<T> generator;

    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));

    dataset.write(mat);

    // read it back
    Matrix_rm result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_rm_cm_matrix, T, numerical_test_types) {

    eigen_rm_cm_matrix_Test<T>();
}


template<typename T>
void eigen_rm_slice_matrix_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrix_cm;
    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix_rm;
    std::ostringstream filename;
    filename << "h5_rw_matrix_cm_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_rm mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));

    Matrix_rm tmat = mat.block(0, 0, 5, 5);
    dataset.select({0, 0}, {5, 5}).write(tmat);

    tmat = mat.block(0, 5, 5, 5);
    dataset.select({0, 5}, {5, 5}).write(tmat);

    tmat = mat.block(5, 0, 5, 5);
    dataset.select({5, 0}, {5, 5}).write(tmat);

    tmat = mat.block(5, 5, 5, 5);
    dataset.select({5, 5}, {5, 5}).write(tmat);

    // read it back
    Matrix_rm result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_rm_slice_matrix, T, numerical_test_types) {

    eigen_rm_slice_matrix_Test<T>();
}

template<typename T>
void eigen_cm_slice_matrix_Test() {

    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrix_cm;
    typedef typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix_rm;

    std::ostringstream filename;
    filename << "h5_rw_matrix_cm_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_cm mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (Eigen::Index i = 0; i < mat.rows(); ++i) {
        for (Eigen::Index j = 0; j < mat.cols(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace({size_x, size_y}));

    Matrix_cm tmat = mat.block(0, 0, 5, 5);
    dataset.select({0, 0}, {5, 5}).write(tmat);

    tmat = mat.block(0, 5, 5, 5);
    dataset.select({0, 5}, {5, 5}).write(tmat);

    tmat = mat.block(5, 0, 5, 5);
    dataset.select({5, 0}, {5, 5}).write(tmat);

    tmat = mat.block(5, 5, 5, 5);
    dataset.select({5, 5}, {5, 5}).write(tmat);

    // read it back
    Matrix_cm result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(eigen_cm_slice_matrix, T, numerical_test_types) {

    eigen_cm_slice_matrix_Test<T>();
}


#endif


#ifdef H5_USE_BOOST

template <typename T>
void MultiArray3DTest() {

    typedef typename boost::multi_array<T, 3> MultiArray;

    std::ostringstream filename;
    filename << "h5_rw_multiarray_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10, size_z = 10;
    const std::string DATASET_NAME("dset");
    MultiArray array(boost::extents[size_x][size_y][size_z]);

    ContentGenerate<T> generator;
    std::generate(array.data(), array.data() + array.num_elements(), generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset =
        file.createDataSet<T>(DATASET_NAME, DataSpace::From(array));

    dataset.write(array);

    // read it back
    MultiArray result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            for (size_t k = 0; k < size_z; ++k) {
                // std::cout << array[i][j][k] << " ";
                BOOST_CHECK_EQUAL(array[i][j][k], result[i][j][k]);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(MultiArray3D, T, numerical_test_types) {

    MultiArray3DTest<T>();
}

template <typename T>
void ublas_matrix_Test() {

  typedef typename boost::numeric::ublas::matrix<T,boost::numeric::ublas::row_major> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (std::size_t i = 0; i < mat.size1(); ++i) {
        for (std::size_t j = 0; j < mat.size2(); ++j) {
            mat(i, j) = generator();
        }
    }
    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));
    dataset.write(mat);

    // read it back
    Matrix result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ublas_matrix, T, numerical_test_types) {

    ublas_matrix_Test<T>();
}


template <typename T>
void ublas_cm_matrix_Test() {

  typedef typename boost::numeric::ublas::matrix<T,boost::numeric::ublas::column_major> Matrix;

    std::ostringstream filename;
    filename << "h5_rw_matrix_cm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (std::size_t i = 0; i < mat.size1(); ++i) {
        for (std::size_t j = 0; j < mat.size2(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));

    dataset.write(mat);

    // read it back
    Matrix result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ublas_cm_matrix, T, numerical_test_types) {

    ublas_cm_matrix_Test<T>();
}


template<typename T>
void ublas_cm_rm_matrix_Test() {

    typedef typename boost::numeric::ublas::matrix<T, boost::numeric::ublas::column_major> Matrix_cm;
    typedef typename boost::numeric::ublas::matrix<T, boost::numeric::ublas::row_major> Matrix_rm;

    std::ostringstream filename;
    filename << "h5_rw_matrix_cm_rm_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10, size_y = 10;
    const std::string DATASET_NAME("dset");

    Matrix_rm mat(size_x, size_y);

    ContentGenerate<T> generator;
    for (std::size_t i = 0; i < mat.size1(); ++i) {
        for (std::size_t j = 0; j < mat.size2(); ++j) {
            mat(i, j) = generator();
        }
    }

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(mat));

    dataset.write(mat);

    // read it back
    Matrix_cm result;

    dataset.read(result);

    for (size_t i = 0; i < size_x; ++i) {
        for (size_t j = 0; j < size_y; ++j) {
            // std::cout << array[i][j][k] << " ";
            BOOST_CHECK_EQUAL(mat(i, j), result(i, j));
        }
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ublas_cm_rm_matrix, T, numerical_test_types) {

    ublas_cm_rm_matrix_Test<T>();
}



#endif

template <typename T>
void selectionArraySimpleTest() {

    typedef typename std::vector<T> Vector;

    std::ostringstream filename;
    filename << "h5_rw_select_test_" << typeid(T).name() << "_test.h5";

    const size_t size_x = 10;
    const size_t offset_x = 2, count_x = 5;

    const std::string DATASET_NAME("dset");

    Vector values(size_x);

    ContentGenerate<T> generator;
    std::generate(values.begin(), values.end(), generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    DataSet dataset =
        file.createDataSet<T>(DATASET_NAME, DataSpace::From(values));

    dataset.write(values);

    file.flush();

    // select slice
    {
        // read it back
        Vector result;
        std::vector<size_t> offset;
        offset.push_back(offset_x);
        std::vector<size_t> size;
        size.push_back(count_x);

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
        std::vector<size_t> ids;
        ids.push_back(1);
        ids.push_back(3);
        ids.push_back(4);
        ids.push_back(7);

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

BOOST_AUTO_TEST_CASE_TEMPLATE(selectionArraySimple, T, dataset_test_types) {

    selectionArraySimpleTest<T>();
}

template <typename T>
void columnSelectionTest() {

    std::ostringstream filename;
    filename << "h5_rw_select_column_test_" << typeid(T).name() << "_test.h5";

    const size_t x_size = 10;
    const size_t y_size = 7;

    const std::string DATASET_NAME("dset");

    T values[x_size][y_size];

    ContentGenerate<T> generator;
    generate2D(values, x_size, y_size, generator);

    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims;
    dims.push_back(x_size);
    dims.push_back(y_size);

    DataSpace dataspace(dims);
    // Create a dataset with arbitrary type
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace);

    dataset.write(values);

    file.flush();

    std::vector<size_t> columns;
    columns.push_back(1);
    columns.push_back(3);
    columns.push_back(5);

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
    filename << "h5_rw_attribute_scalar_rw" << typeid(T).name() << "_test.h5";

    File h5file(filename.str(),
                File::ReadWrite | File::Create | File::Truncate);

    ContentGenerate<T> generator;

    const T attribute_value(generator());

    Group g = h5file.createGroup("metadata");

    bool family_exist = g.hasAttribute("family");
    BOOST_CHECK_EQUAL(family_exist, false);

    // write a scalar attribute
    {
        T out(attribute_value);
        Attribute att = g.createAttribute<T>("family", DataSpace::From(out));
        att.write(out);
    }

    h5file.flush();

    // test if attribute exist
    family_exist = g.hasAttribute("family");
    BOOST_CHECK_EQUAL(family_exist, true);

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
