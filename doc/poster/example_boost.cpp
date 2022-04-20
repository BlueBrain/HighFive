#include <complex>

#define H5_USE_BOOST 1

#include <boost/multi_array.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

using complex_t = std::complex<double>;

void data_io() {
    boost::multi_array<complex_t, 4> multi_array(boost::extents[3][2][1][1]);
    std::fill_n(multi_array.origin(), multi_array.num_elements(), 1.0);
    multi_array[1][1][0][0] = complex_t{1.1, 1.2};

    HighFive::File file("multi_array_complex.h5", HighFive::File::Truncate);

    HighFive::DataSet dataset =
        file.createDataSet<complex_t>("multi_array", HighFive::DataSpace::From(multi_array));

    dataset.write(multi_array);
}
