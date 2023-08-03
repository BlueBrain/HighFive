#include <xtensor/xarray.hpp>

#include <highfive/H5Easy.hpp>

int main() {
    xt::xarray<int> A = xt::ones<int>({10, 3});

    // open a file
    H5Easy::File file("tmp.h5", H5Easy::File::Overwrite);

    // write dataset (automatically creates groups if needed)
    H5Easy::dump(file, "/path/to/A", A);

    // read from dataset
    auto B = H5Easy::load<xt::xarray<int>>(file, "/path/to/A");

    // write attribute
    H5Easy::dumpAttribute(file, "/path/to/A", "date", std::string("today"));

    // read from attribute
    auto d = H5Easy::loadAttribute<std::string>(file, "/path/to/A", "date");

    // create extendible dataset and extend it
    for (size_t i = 0; i < 10; ++i) {
        H5Easy::dump(file, "/path/to/extendible", i, {i});
    }

    return 0;
}
