/*
 *  Copyright (c), 2021, Blue Brain Project, EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

// Compound datatype test :: May 2021
// //////////////////////////////////

#include <highfive/highfive.hpp>


typedef struct {
    double width;
    double height;
} Size2D;


HighFive::CompoundType create_compound_Size2D() {
    return {{"width", HighFive::create_datatype<double>()},
            {"height", HighFive::create_datatype<double>()}};
}

HIGHFIVE_REGISTER_TYPE(Size2D, create_compound_Size2D)

int main() {
    const std::string dataset_name("dims");

    HighFive::File file("compounds_test.h5", HighFive::File::Truncate);

    auto t1 = create_compound_Size2D();
    t1.commit(file, "Size2D");

    std::vector<Size2D> dims = {{1., 2.5}, {3., 4.5}};
    auto dataset = file.createDataSet(dataset_name, dims);

    auto g1 = file.createGroup("group1");
    g1.createAttribute(dataset_name, dims);
}
