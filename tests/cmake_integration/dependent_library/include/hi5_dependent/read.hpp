#pragma once

#include <highfive/highfive.hpp>
#include <vector>

#if HI5_DEPENDENT_HAS_BOOST == 1
#include <highfive/boost.hpp>
#endif

namespace hi5_dependent {
std::vector<double> read_vector(const HighFive::DataSet& dset);

#if HI5_DEPENDENT_HAS_BOOST == 1
boost::numeric::ublas::matrix<double> read_boost(const HighFive::DataSet& dset);
HighFive::DataSet write_boost(HighFive::File& file, const boost::numeric::ublas::matrix<double>& x);
#endif
}  // namespace hi5_dependent
