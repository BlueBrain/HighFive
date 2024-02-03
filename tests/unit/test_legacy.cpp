// This file collects tests the require legacy behaviour of v2 (and older) to
// pass. Tests in this file could be bugs too.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <highfive/highfive.hpp>

using namespace HighFive;

TEST_CASE("HighFiveReadWriteConsts") {
    // This test seems really strange. Essentially, it malloc's a 3**3 doubles.
    // Then reinterpret_cast's the pointer to the first double (a `double *`)
    // as a `double***`. And then uses `inspector` based code to write from the
    // `double***`.

    const std::string file_name("3d_dataset_from_flat.h5");
    const std::string dataset_name("dset");
    const std::array<std::size_t, 3> DIMS{3, 3, 3};
    using datatype = int;

    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    DataSpace dataspace = DataSpace(DIMS);

    DataSet dataset = file.createDataSet<datatype>(dataset_name, dataspace);
    std::vector<datatype> const t1(DIMS[0] * DIMS[1] * DIMS[2], 1);
    auto raw_3d_vec_const = reinterpret_cast<datatype const* const* const*>(t1.data());
    dataset.write_raw(raw_3d_vec_const);

    std::vector<std::vector<std::vector<datatype>>> result;
    dataset.read(result);
    for (const auto& vec2d: result) {
        for (const auto& vec1d: vec2d) {
            REQUIRE(vec1d == (std::vector<datatype>{1, 1, 1}));
        }
    }
}
