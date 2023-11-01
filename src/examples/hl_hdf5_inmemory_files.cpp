/*
 *  Copyright (c), 2022, Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <vector>

#include <highfive/highfive.hpp>

#include <hdf5_hl.h>

using namespace HighFive;

class InMemoryFile: public HighFive::File {
  public:
    explicit InMemoryFile(std::vector<std::uint8_t> buffer)
        : _buffer(std::move(buffer)) {
        _hid = H5LTopen_file_image(_buffer.data(),
                                   sizeof(_buffer[0]) * _buffer.size(),
                                   H5LT_FILE_IMAGE_DONT_RELEASE | H5LT_FILE_IMAGE_DONT_COPY);
    }

  private:
    std::vector<std::uint8_t> _buffer;
};


// Create a 2D dataset 10x3 of double with eigen matrix
// and write it to a file
int main(void) {
    const std::string file_name("inmemory_file.h5");
    const std::string dataset_name("dset");

    auto data = std::vector<double>{1.0, 2.0, 3.0};

    {
        // We create an HDF5 file.
        File file(file_name, File::Truncate);
        file.createDataSet(dataset_name, data);
    }

    // Simulate having an inmemory file by reading a file
    // byte-by-byte into RAM.
    auto buffer = std::vector<std::uint8_t>(1ul << 20);
    auto file = std::fopen(file_name.c_str(), "r");
    auto nread = std::fread(buffer.data(), sizeof(buffer[0]), buffer.size(), file);
    std::cout << "Bytes read: " << nread << "\n";

    // Create a file from a buffer.
    auto h5 = InMemoryFile(std::move(buffer));

    // Read a dataset as usual.
    auto read_back = h5.getDataSet(dataset_name).read<std::vector<double>>();

    // Check if the values match.
    for (size_t i = 0; i < read_back.size(); ++i) {
        if (read_back[i] != data[i]) {
            throw std::runtime_error("Values don't match.");
        } else {
            std::cout << "read_back[" << i << "] = " << read_back[i] << "\n";
        }
    }

    return 0;
}
