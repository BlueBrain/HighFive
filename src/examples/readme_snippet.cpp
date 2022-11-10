#include <highfive/H5File.hpp>

using namespace HighFive;

int main() {
    std::string filename = "/tmp/new_file.h5";

    {
        // We create an empty HDF55 file, by truncating an existing
        // file if required:
        File file(filename, File::Truncate);

        std::vector<int> data(50, 1);
        file.createDataSet("grp/data", data);
    }

    {
        // We open the file as read-only:
        File file(filename, File::ReadOnly);
        auto dataset = file.getDataSet("grp/data");

        // Read back, with allocating:
        auto data = dataset.read<std::vector<int>>();

        // Because `data` has the correct size, this will
        // not cause `data` to be reallocated:
        dataset.read(data);
    }

    return 0;
}
