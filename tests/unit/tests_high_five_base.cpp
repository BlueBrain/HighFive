/*
 * @file tests_mvd2.cpp
 *
 * @brief unit tests for mvd2 parser
 *
 *
 **/


#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <typeinfo>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5Group.hpp>

#define BOOST_TEST_MAIN HighFiveTest
#include <boost/test/included/unit_test.hpp>

using namespace HighFive;

typedef boost::mpl::list<int, unsigned int, long, unsigned long, unsigned char, char, float, double, long long, unsigned long long> numerical_test_types;
typedef boost::mpl::list<int, unsigned int, long, unsigned long, unsigned char, char, float, double, std::string> dataset_test_types;

template<typename T, typename Func>
void generate2D(T* table, size_t x, size_t y, Func & func){
    for(size_t i = 0; i < x; i++){
        for(size_t j = 0; j < y; j++){
            table[i][j] = func();
        }
    }
}

template<typename T, typename Func>
void generate2D(std::vector<std::vector<T> > & vec, size_t x, size_t y, Func & func){
    vec.resize(x);
    for(size_t i = 0; i < x; i++){
        vec[i].resize(y);
        for(size_t j = 0; j < y; j++){
            vec[i][j] = func();
        }
    }
}


template<typename T>
struct ContentGenerate{
    ContentGenerate(T init_val = T(0), T inc_val = T(1) + T(1)/T(10) ) : _init(init_val), _inc(inc_val){}

    T operator()(){
        T ret =_init;
        _init += _inc;
        return ret;
    }

    T _init, _inc;
};


template<>
struct ContentGenerate<char>{
    ContentGenerate() : _init('a'){}

    char operator()(){
        char ret =_init;
        if(++_init >= 0x61+26)
            _init = 0x61;
        return ret;
    }

    char _init;
};


template<>
struct ContentGenerate<std::string>{
    ContentGenerate(){}

    std::string operator()(){
        ContentGenerate<char> gen;
        std::string random_string;
        const size_t size_string = std::rand()%1000;
        random_string.resize(size_string);
        std::generate(random_string.begin(), random_string.end(), gen);
        return random_string;
    }

};




BOOST_AUTO_TEST_CASE( HighFiveBasic )
{

    const std::string FILE_NAME("h5tutr_dset.h5");
    const std::string DATASET_NAME("dset");


    // Create a new file using the default property lists.
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

    // Create the data space for the dataset.
    std::vector<size_t> dims;
    dims.push_back(4);
    dims.push_back(6);

    DataSpace dataspace(dims);

    // Create a dataset with double precision floating points
    DataSet dataset_double = file.createDataSet(DATASET_NAME + "_double", dataspace, AtomicType<double>());

    {
        BOOST_CHECK_THROW({
            DataSet fail_duplicated = file.createDataSet(DATASET_NAME + "_double", dataspace, AtomicType<double>());
        },
        DataSetException);
    }

    DataSet dataset_size_t = file.createDataSet<size_t>(DATASET_NAME + "_size_t",  dataspace);

}

BOOST_AUTO_TEST_CASE( HighFiveException )
{

    // Create a new file
    File file1("random_file_123", File::ReadWrite | File::Create | File::Truncate);

    BOOST_CHECK_THROW({
    // triggers a file creation conflict
        File file2("random_file_123", File::ReadWrite | File::Create);
     }, FileException);


}


BOOST_AUTO_TEST_CASE( HighFiveGroupAndDataSet )
{

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
        Group g1 =  file.createGroup(GROUP_NAME2);
        // relative group
        Group nested = g1.createGroup(GROUP_NESTED_NAME);

        // Create the data space for the dataset.
        std::vector<size_t> dims;
        dims.push_back(4);
        dims.push_back(6);

        DataSpace dataspace(dims);

        DataSet dataset_absolute = file.createDataSet(GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" + DATASET_NAME, dataspace, AtomicType<double>());

        DataSet dataset_relative = nested.createDataSet(DATASET_NAME, dataspace, AtomicType<double>());
    }
    // read it back
    {
        File file(FILE_NAME, File::ReadOnly);
        Group g1 = file.getGroup(GROUP_NAME1);
        Group g2 = file.getGroup(GROUP_NAME2);
        Group nested_group2 = g2.getGroup(GROUP_NESTED_NAME);

        DataSet dataset_absolute = file.getDataSet(GROUP_NAME1 + "/" + GROUP_NESTED_NAME + "/" + DATASET_NAME);
        BOOST_CHECK_EQUAL(4, dataset_absolute.getSpace().getDimensions()[0]);

        DataSet dataset_relative = nested_group2.getDataSet(DATASET_NAME);
        BOOST_CHECK_EQUAL(4, dataset_relative.getSpace().getDimensions()[0]);
    }
}



BOOST_AUTO_TEST_CASE( DataTypeEqualSimple )
{

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

BOOST_AUTO_TEST_CASE( DataTypeEqualTakeBack )
{

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
    DataSet dataset = file.createDataSet<size_t>(DATASET_NAME + "_double", dataspace);


    AtomicType<size_t> s;
    AtomicType<double> d;


    BOOST_CHECK(s == dataset.getDataType());
    BOOST_CHECK(d != dataset.getDataType());


}




template<typename T>
void readWrite2DArrayTest()
{

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

    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, dataspace);




    T array[x_size][y_size];

    ContentGenerate<T> generator;
    generate2D(array, x_size, y_size, generator);

    dataset.write(array);

    T result[x_size][y_size];

    dataset.read(result);

    for(size_t i =0; i < x_size; ++i){
        for(size_t j =0; j < y_size; ++j){
            BOOST_CHECK_EQUAL(result[i][j], array[i][j]);
        }
    }
}



BOOST_AUTO_TEST_CASE_TEMPLATE( ReadWrite2DArray, T, numerical_test_types){

    readWrite2DArrayTest<T>();
}

template <typename T>
void readWriteVectorTest(){
    using namespace HighFive;

    std::ostringstream filename;
    filename << "h5_rw_vec_" << typeid(T).name() << "_test.h5";

    std::srand(std::time(0));
    const size_t x_size = 800;
    const std::string DATASET_NAME("dset");
    typename std::vector<T> vec;


    // Create a new file using the default property lists.
    File file(filename.str(), File::ReadWrite | File::Create | File::Truncate);

    vec.resize(x_size);
    ContentGenerate<T> generator;

    std::generate(vec.begin(), vec.end(), generator);

    // Create a dataset with double precision floating points
    DataSet dataset = file.createDataSet<T>(DATASET_NAME, DataSpace::From(vec));


    dataset.write(vec);

    typename std::vector<T> result;

    dataset.read(result);

    BOOST_CHECK_EQUAL(vec.size(), x_size);
    BOOST_CHECK_EQUAL(result.size(), x_size);

    for(size_t i =0; i < x_size; ++i){
        //std::cout << result[i] << " " << vec[i] << "  ";
        BOOST_CHECK_EQUAL(result[i], vec[i]);
    }
}



BOOST_AUTO_TEST_CASE_TEMPLATE( ReadWriteDataSetVector, T, dataset_test_types)
{
    readWriteVectorTest<T>();
}



template <typename T>
void readWriteVector2DTest(){
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

    for(size_t i =0; i < x_size; ++i){
        for(size_t j =0; i < y_size; ++i){
            BOOST_CHECK_EQUAL(result[i][j], vec[i][j]);
        }
    }
}



BOOST_AUTO_TEST_CASE_TEMPLATE( readWriteVector2D, T, numerical_test_types){

    readWriteVector2DTest<T>();
}

