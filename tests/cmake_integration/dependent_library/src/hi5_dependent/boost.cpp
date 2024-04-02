#include <hi5_dependent/read.hpp>
#include <hi5_dependent/write.hpp>

#include <boost/numeric/ublas/matrix.hpp>

namespace hi5_dependent {

boost::numeric::ublas::matrix<double> read_boost(const HighFive::DataSet& dset) {
    return dset.read<boost::numeric::ublas::matrix<double>>();
}

HighFive::DataSet write_boost(HighFive::File& file,
                              const boost::numeric::ublas::matrix<double>& x) {
    return file.createDataSet("foo", x);
}

}  // namespace hi5_dependent
