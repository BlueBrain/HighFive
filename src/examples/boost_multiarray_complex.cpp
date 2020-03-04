#include <complex>

#undef H5_USE_BOOST
#define H5_USE_BOOST

#include <boost/multi_array.hpp>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>


typedef std::complex<double> complex_t;

int main()
{
    boost::multi_array<complex_t, 4> multi_array( boost::extents[3][2][1][1] );
    std::fill( multi_array.origin(),  multi_array.origin() + multi_array.num_elements(), 1.0 );

    HighFive::File file( "ma.h5" , HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);

    HighFive::DataSet dataset = file.createDataSet<complex_t>( "multi_array", HighFive::DataSpace::From( multi_array ));
    dataset.write(multi_array);

    return 0;
}
