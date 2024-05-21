/*
 *  Copyright (c), 2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#if HIGHFIVE_TEST_BOOST
#include <string>

#include <catch2/catch_template_test_macros.hpp>

#include <highfive/highfive.hpp>
#include <highfive/boost.hpp>

using namespace HighFive;

TEST_CASE("Test boost::multi_array with fortran_storage_order") {
    const std::string file_name("h5_multi_array_fortran.h5");
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    boost::multi_array<int, 2> ma(boost::extents[2][2], boost::fortran_storage_order());
    auto dset = file.createDataSet<int>("main_dset", DataSpace::From(ma));
    CHECK_THROWS_AS(dset.write(ma), DataTypeException);
}
#endif
