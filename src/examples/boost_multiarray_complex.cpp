/*
 *  Copyright (c), 2020 Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <algorithm>
#include <complex>

#undef H5_USE_BOOST
#define H5_USE_BOOST

#include <boost/multi_array.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>

typedef std::complex<double> complex_t;

int main() {
    boost::multi_array<complex_t, 4> multi_array(boost::extents[3][2][1][1]);
    std::fill_n(multi_array.origin(), multi_array.num_elements(), 1.0);
    multi_array[1][1][0][0] = complex_t{1.1, 1.2};

    HighFive::File file("multi_array_complex.h5", HighFive::File::Truncate);

    HighFive::DataSet dataset = file.createDataSet("multi_array", multi_array);
    return 0;
}
