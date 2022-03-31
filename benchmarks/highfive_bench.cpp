#include <vector>
#include <highfive/H5File.hpp>

const std::vector<std::vector<int>> data(1000000, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

int main() {
    HighFive::File("dataset_integer.h5", HighFive::File::Truncate)
        .createDataSet("dataset", data);
}
