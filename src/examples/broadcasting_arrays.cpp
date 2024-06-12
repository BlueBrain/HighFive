#include <highfive/highfive.hpp>

// This example explains how to read a dataset with some shape into an array of
// some other shape. Naturally, this only makes sense if the number of elements
// doesn't change.
//
// Note that due to how HDF5 works, writing from one shape into some other
// shape is expected to work automatically.
//
// Same is true for reading. However, HighFive also allocates memory, the array
// into which the data is read is forced to have the same shape as the
// memspace. When performing selections it can often happen that one selects a
// one-dimensional slice from a higher dimensional array.  In this case we want
// to be able to read into a one dimensional array, e.g. `std::vector<double>`.
//
// Broadcasting is a common technique for hiding benign differences in
// dimensionality. In HighFive we suggest to either "squeeze" or "reshape" the
// memspace, rather than broadcasting. This example demonstrates the required
// syntax.
//
// Note: These techniques can also be used for general hyperslabs which the
// user knows are in fact hypercubes, i.e. regular.
//
// Note: HighFive v2 has support for broadcasting; but because it's quirky,
// less powerful than the demonstrated technique, relied on a compile-time
// constant rank and is quite complex to maintain, the functionality was
// removed from v3.

using namespace HighFive;

int main(void) {
    File file("broadcasting_arrays.h5", File::Truncate);

    std::vector<size_t> dims{3, 1};
    std::vector<double> values{1.0, 2.0, 3.0};

    auto dset = file.createDataSet("dset", DataSpace(dims), create_datatype<double>());

    // Note that because `values` is one-dimensional, we can't write it
    // to a dataset of dimensions `[3, 1]` directly. Instead we use:
    dset.squeezeMemSpace({1}).write(values);

    // When reading, (re-)allocation might occur. The shape to be allocated is
    // the dimensions of the memspace. Therefore, one might want to either remove
    // an axis:
    dset.squeezeMemSpace({1}).read(values);

    // or reshape the memspace:
    dset.reshapeMemSpace({3}).read(values);
}
