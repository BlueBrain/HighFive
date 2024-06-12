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

TEST_CASE("Array of char pointers") {
    // Currently, serializing an `std::vector<char*>` as
    // fixed or variable length strings doesn't work.
    //
    // This isn't a test of correctness. Rather it asserts the fact that
    // something doesn't work in HighFive. Knowing it doesn't work is useful
    // for developers, but could change in the future.

    const std::string file_name = "vector_char_pointer.h5";

    File file(file_name, File::Truncate);

    size_t n_strings = 3;
    size_t n_chars = 4;
    char storage[3][4] = {"foo", "bar", "000"};
    auto strings = std::vector<char*>(n_strings);

    for (size_t i = 0; i < n_strings; ++i) {
        strings[i] = static_cast<char*>(storage[i]);
    }

    auto filespace = DataSpace({n_strings});

    SECTION("fixed length") {
        auto datatype = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
        auto dset = file.createDataSet("dset", filespace, datatype);
        REQUIRE_THROWS(dset.write(strings));
    }

    SECTION("variable length") {
        auto datatype = VariableLengthStringType();
        auto dset = file.createDataSet("dset", filespace, datatype);
        REQUIRE_THROWS(dset.write(strings));
    }
}
