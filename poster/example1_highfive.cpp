#include <highfive/H5File.hpp>

using HighFive::File;

void write_io() {
    std::vector<int> d1(50, 1);

    // Open a file
    File file("tmp.h5", File::ReadWrite | File::Truncate);

    // Create DataSet and write data (short form)
    file.createDataSet("/group/dset1", d1);

    // Read the data
    std::vector<int> d1_read;
    file.getDataSet("/group/dset1").read(d1_read);
}
