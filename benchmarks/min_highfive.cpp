#include <vector>
#include <highfive/H5File.hpp>

int main() {
    HighFive::File("dataset_integer.h5", HighFive::File::Truncate)
        .createDataSet("dataset", std::vector<int>{1, 2, 3, 4});
    return 0;
}
