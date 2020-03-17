#ifndef H5_TEST_SIMPLETON_HPP
#define H5_TEST_SIMPLETON_HPP

// Include all headers here to catch any missing `inline` statements, since
// they will be included by two different compilation units.
#include <highfive/H5Attribute.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5DataType.hpp>
#include <highfive/H5Easy.hpp>
#include <highfive/H5Exception.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5FileDriver.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Object.hpp>
#include <highfive/H5PropertyList.hpp>
#include <highfive/H5Reference.hpp>
#include <highfive/H5Selection.hpp>
#include <highfive/H5Utility.hpp>

// Boost should always be found in this setup
#include <boost/numeric/ublas/matrix.hpp>

void function(const HighFive::Object& obj);
void other_function(const boost::numeric::ublas::matrix<double>& m);

#endif
