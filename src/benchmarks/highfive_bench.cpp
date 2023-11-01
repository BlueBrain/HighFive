#include <highfive/highfive.hpp>
#include <vector>

const std::vector<std::vector<int>> data(1000000, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

int do_iteration() {
    HighFive::File("dataset_integer.h5", HighFive::File::Truncate).createDataSet("dataset", data);
    return 0;
}

int main() {
    for (int i = 0; i < 200; i++) {
        do_iteration();
    }
}
