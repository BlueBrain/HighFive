/*
 *  Copyright (c), 2024, Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

// This example demonstrates using `std::span`. An `std::span` is a pointer
// with a size.

#include <string>
#include <vector>

#include <highfive/highfive.hpp>

#include <highfive/span.hpp>

int main(void) {
    using namespace HighFive;

    std::string file_name = "read_write_span.h5";
    std::string dataset_name = "array";

    File file(file_name, File::Truncate);

    // Let's write to file.
    {
        // Assume we have one-dimensional data in some unsupported format (we
        // use `std::vector` for simplicity). Further, assume that the data is
        // stored contiguously. Then one can create an `std::span`.
        std::vector<double> values{1.0, 2.0, 3.0};
        auto view = std::span<double>(values.data(), values.size());

        // Given the span, HighFive can deduce the shape of the dataset. Hence,
        // spans are fully supported when writing. For example:
        auto dataset = file.createDataSet(dataset_name, view);
    }

    // Let's read from file.
    {
        auto dataset = file.getDataSet(dataset_name);

        // Since spans are views, HighFive can't (or wont) allocate memory.
        // Instead one must preallocate memory and then create a span for that
        // memory:
        auto values = std::vector<double>(dataset.getElementCount());
        auto view = std::span<double>(values.data(), values.size());

        // ... now we can read into the preallocated memory:
        dataset.read(view);
    }

    return 0;
}
