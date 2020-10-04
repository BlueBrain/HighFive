/*
 *  Copyright (c), 2017-2019, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <typeinfo>
#include <vector>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Reference.hpp>
#include <highfive/H5Utility.hpp>

#define BOOST_TEST_MAIN HighFiveTestBase
#include <boost/test/unit_test.hpp>

#include "tests_high_five.hpp"

using namespace HighFive;


struct Foo {
    int bar;
};

BOOST_AUTO_TEST_CASE(HighFiveConverters) {
    BOOST_REQUIRE(details::h5_continuous<std::vector<int>>::value);
    BOOST_REQUIRE(details::h5_continuous<std::vector<Foo>>::value);
    BOOST_REQUIRE(details::h5_non_continuous<std::vector<std::string>>::value);
    BOOST_REQUIRE(details::h5_non_continuous<std::vector<std::vector<int>>>::value);
}
