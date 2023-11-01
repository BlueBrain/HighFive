/*
 *  Copyright (c), 2022, Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */


#include <iostream>
#include <string>
#include <vector>

#include <highfive/highfive.hpp>

// This example requires HDF5 version 1.10.1 or newer.
#if H5_VERSION_GE(1, 10, 1)

// This example show how to create an HDF5 file that internally aggregates
// metadata and raw data into separate pages. The advantage of this approach
// is that reading a single page, pulls in the metadata for a large chunk of
// the file.
//
// This can be very useful when dealing with many small datasets. Note, this
// is an optimization. Therefore, you must perform measurements in order to
// know if this should be used.
//
// Internally, it uses two free space managers, one for metadata and one for
// raw data. When space for data is allocated, the corresponding free space
// manager is asked to allocate space. It will look if there is enough space
// on a partially filled paged, if yes it keeps filling the page, if not it
// requests page aligned space from the file driver as needed. Upstream
// documentation explains the details well in:
//
//    RFC: HDF5 File Space Management: Paged Aggregation

int main() {
    using namespace HighFive;

    // Create a new file requesting paged allocation.
    auto create_props = FileCreateProps{};

    // Let request pagesizes of 16 kB. This setting should be tuned
    // in real applications. We'll allow HDF5 to not keep track of
    // left-over free space of size less than 128 bytes. Finally,
    // we don't need the free space manager to be stored in the
    // HDF5 file.
    size_t pagesize = 16 * 1024;  // Must be tuned.
    size_t threshold = 128;
    size_t persist = false;

    create_props.add(FileSpaceStrategy(H5F_FSPACE_STRATEGY_PAGE, persist, threshold));
    create_props.add(FileSpacePageSize(pagesize));

    File file("create_page_allocated_files.h5", File::Truncate, create_props);

    // The `file` (and also the low-level `file.getId()`) behave as normal, i.e.
    // one can proceed to add content to the file as usual.

    auto data = std::vector<double>{0.0, 1.0, 2.0};
    file.createDataSet("data", data);

    return 0;
}
#else
#include <iostream>
int main() {
    std::cout << "This example can't be run prior to HDF5 1.10.1.\n";
    return 0;
}
#endif
