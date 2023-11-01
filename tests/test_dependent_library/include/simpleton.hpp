#ifndef H5_TEST_SIMPLETON_HPP
#define H5_TEST_SIMPLETON_HPP

// Include all headers here to catch any missing `inline` statements, since
// they will be included by two different compilation units.
#include <highfive/highfive.hpp>

// Boost should always be found in this setup
#include <boost/numeric/ublas/matrix.hpp>

void function(const HighFive::Object& obj);
void other_function(const boost::numeric::ublas::matrix<double>& m);

#endif
