#include <iostream>

#define H5_USE_BOOST 1
#include <highfive/highfive.hpp>

// In some versions of Boost (starting with 1.64), you have to
// include the serialization header before ublas
#include <boost/serialization/vector.hpp>

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

using namespace HighFive;

void data_io() {
    const std::string DATASET_NAME("dset");
    const size_t size_x = 10;
    const size_t size_y = 10;

    try {
        typedef typename boost::numeric::ublas::matrix<double> Matrix;

        // create a 10x10 matrix
        Matrix mat(size_x, size_y);

        // fill it
        for (std::size_t i = 0; i < size_x; ++i) {
            mat(i, i) = static_cast<double>(i);
        }

        // Create a new HDF5 file
        File file("boost_ublas.h5", File::ReadWrite | File::Create | File::Truncate);

        DataSet dataset = file.createDataSet<double>(DATASET_NAME, DataSpace::From(mat));

        dataset.write(mat);

        Matrix result;
        dataset.read(result);

        std::cout << "Matrix result:\n" << result << std::endl;

    } catch (const Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }
}
