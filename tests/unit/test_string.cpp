/*
 *  Copyright (c), 2017-2024, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <highfive/highfive.hpp>
#include "tests_high_five.hpp"
#include "create_traits.hpp"


using namespace HighFive;
using Catch::Matchers::Equals;

TEST_CASE("StringType") {
    SECTION("enshrine-defaults") {
        auto fixed_length = FixedLengthStringType(32, StringPadding::SpacePadded);
        auto variable_length = VariableLengthStringType();

        REQUIRE(fixed_length.getCharacterSet() == CharacterSet::Ascii);
        REQUIRE(variable_length.getCharacterSet() == CharacterSet::Ascii);
    }

    SECTION("fixed-length") {
        auto fixed_length =
            FixedLengthStringType(32, StringPadding::SpacePadded, CharacterSet::Utf8);
        auto string_type = fixed_length.asStringType();

        REQUIRE(string_type.getId() == fixed_length.getId());
        REQUIRE(string_type.getCharacterSet() == CharacterSet::Utf8);
        REQUIRE(string_type.getPadding() == StringPadding::SpacePadded);
        REQUIRE(string_type.getSize() == 32);
        REQUIRE(!string_type.isVariableStr());
        REQUIRE(string_type.isFixedLenStr());
    }

    SECTION("variable-length") {
        auto variable_length = VariableLengthStringType(CharacterSet::Utf8);
        auto string_type = variable_length.asStringType();

        REQUIRE(string_type.getId() == variable_length.getId());
        REQUIRE(string_type.getCharacterSet() == CharacterSet::Utf8);
        REQUIRE(string_type.isVariableStr());
        REQUIRE(!string_type.isFixedLenStr());
    }

    SECTION("atomic") {
        auto atomic = AtomicType<double>();
        REQUIRE_THROWS(atomic.asStringType());
    }
}

template <class CreateTraits>
void check_single_string(File file, size_t string_length) {
    auto value = std::string(string_length, 'o');
    auto dataspace = DataSpace::From(value);

    auto n_chars = value.size() + 1;
    auto n_chars_overlength = n_chars + 10;
    auto fixed_length = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
    auto overlength_nullterm = FixedLengthStringType(n_chars_overlength,
                                                     StringPadding::NullTerminated);
    auto overlength_nullpad = FixedLengthStringType(n_chars_overlength, StringPadding::NullPadded);
    auto overlength_spacepad = FixedLengthStringType(n_chars_overlength,
                                                     StringPadding::SpacePadded);
    auto variable_length = VariableLengthStringType();

    SECTION("automatic") {
        auto obj = CreateTraits::create(file, "auto", value);
        REQUIRE(obj.template read<std::string>() == value);
    }

    SECTION("fixed length") {
        auto obj = CreateTraits::create(file, "fixed", dataspace, fixed_length);
        obj.write(value);
        REQUIRE(obj.template read<std::string>() == value);
    }

    SECTION("overlength null-terminated") {
        auto obj =
            CreateTraits::create(file, "overlength_nullterm", dataspace, overlength_nullterm);
        obj.write(value);
        REQUIRE(obj.template read<std::string>() == value);
    }

    SECTION("overlength null-padded") {
        auto obj = CreateTraits::create(file, "overlength_nullpad", dataspace, overlength_nullpad);
        obj.write(value);
        auto expected = std::string(n_chars_overlength, '\0');
        expected.replace(0, value.size(), value.data());
        REQUIRE(obj.template read<std::string>() == expected);
    }

    SECTION("overlength space-padded") {
        auto obj =
            CreateTraits::create(file, "overlength_spacepad", dataspace, overlength_spacepad);
        obj.write(value);
        auto expected = std::string(n_chars_overlength, ' ');
        expected.replace(0, value.size(), value.data());
        REQUIRE(obj.template read<std::string>() == expected);
    }

    SECTION("variable length") {
        auto obj = CreateTraits::create(file, "variable", dataspace, variable_length);
        obj.write(value);
        REQUIRE(obj.template read<std::string>() == value);
    }
}

template <class CreateTraits>
void check_multiple_string(File file, size_t string_length) {
    using value_t = std::vector<std::string>;
    auto value = value_t{std::string(string_length, 'o'), std::string(string_length, 'x')};

    auto dataspace = DataSpace::From(value);

    auto string_overlength = string_length + 10;
    auto onpoint_nullpad = FixedLengthStringType(string_length, StringPadding::NullPadded);
    auto onpoint_spacepad = FixedLengthStringType(string_length, StringPadding::SpacePadded);

    auto overlength_nullterm = FixedLengthStringType(string_overlength,
                                                     StringPadding::NullTerminated);
    auto overlength_nullpad = FixedLengthStringType(string_overlength, StringPadding::NullPadded);
    auto overlength_spacepad = FixedLengthStringType(string_overlength, StringPadding::SpacePadded);
    auto variable_length = VariableLengthStringType();

    auto check = [](const value_t actual, const value_t& expected) {
        REQUIRE(actual.size() == expected.size());
        for (size_t i = 0; i < actual.size(); ++i) {
            REQUIRE(actual[i] == expected[i]);
        }
    };

    SECTION("automatic") {
        auto obj = CreateTraits::create(file, "auto", value);
        check(obj.template read<value_t>(), value);
    }

    SECTION("variable length") {
        auto obj = CreateTraits::create(file, "variable", dataspace, variable_length);
        obj.write(value);
        check(obj.template read<value_t>(), value);
    }

    auto make_padded_reference = [&](char pad, size_t n) {
        auto expected = std::vector<std::string>(value.size(), std::string(n, pad));
        for (size_t i = 0; i < value.size(); ++i) {
            expected[i].replace(0, value[i].size(), value[i].data());
        }

        return expected;
    };

    auto check_fixed_length = [&](const std::string& label, size_t length) {
        SECTION(label + " null-terminated") {
            auto datatype = FixedLengthStringType(length + 1, StringPadding::NullTerminated);
            auto obj = CreateTraits::create(file, label + "_nullterm", dataspace, datatype);
            obj.write(value);
            check(obj.template read<value_t>(), value);
        }

        SECTION(label + " null-padded") {
            auto datatype = FixedLengthStringType(length, StringPadding::NullPadded);
            auto obj = CreateTraits::create(file, label + "_nullpad", dataspace, datatype);
            obj.write(value);
            auto expected = make_padded_reference('\0', length);
            check(obj.template read<value_t>(), expected);
        }

        SECTION(label + " space-padded") {
            auto datatype = FixedLengthStringType(length, StringPadding::SpacePadded);
            auto obj = CreateTraits::create(file, label + "_spacepad", dataspace, datatype);
            obj.write(value);
            auto expected = make_padded_reference(' ', length);
            check(obj.template read<value_t>(), expected);
        }
    };

    check_fixed_length("onpoint", string_length);
    check_fixed_length("overlength", string_length + 5);


    SECTION("underlength null-terminated") {
        auto datatype = FixedLengthStringType(string_length, StringPadding::NullTerminated);
        auto obj = CreateTraits::create(file, "underlength_nullterm", dataspace, datatype);
        REQUIRE_THROWS(obj.write(value));
    }

    SECTION("underlength nullpad") {
        auto datatype = FixedLengthStringType(string_length - 1, StringPadding::NullPadded);
        auto obj = CreateTraits::create(file, "underlength_nullpad", dataspace, datatype);
        REQUIRE_THROWS(obj.write(value));
    }

    SECTION("underlength spacepad") {
        auto datatype = FixedLengthStringType(string_length - 1, StringPadding::NullTerminated);
        auto obj = CreateTraits::create(file, "underlength_spacepad", dataspace, datatype);
        REQUIRE_THROWS(obj.write(value));
    }
}

template <class CreateTraits>
void check_supposedly_nullterm(HighFive::File& file, size_t string_length) {
    auto dataspace = HighFive::DataSpace::Scalar();
    auto datatype = HighFive::FixedLengthStringType(string_length,
                                                    HighFive::StringPadding::NullTerminated);
    auto obj = CreateTraits::create(file,
                                    "not_null_terminated_" + std::to_string(string_length),
                                    dataspace,
                                    datatype);

    // Creates an `string_length` byte, "null-terminated", fixed-length string. The first
    // `string_length` bytes are filled with "a"s. Clearly, this isn't null-terminated. However,
    // h5py will read it back, HDF5 allows us to create these; and they're
    // found in the wild.
    std::string value(string_length, 'a');
    obj.write_raw(value.c_str(), datatype);

    auto actual = obj.template read<std::string>();
    REQUIRE(actual == value);
}

template <class CreateTraits>
void check_supposedly_nullterm_scan(HighFive::File& file) {
    for (size_t n = 1; n < 256; ++n) {
        check_supposedly_nullterm<CreateTraits>(file, n);
    }

    check_supposedly_nullterm<CreateTraits>(file, 4091);
    check_supposedly_nullterm<CreateTraits>(file, 4092);
    check_supposedly_nullterm<CreateTraits>(file, 4093);
}

TEST_CASE("HighFiveSTDString (attribute, nullterm cornercase)") {
    auto file = HighFive::File("not_null_terminated_attribute.h5", HighFive::File::Truncate);
    check_supposedly_nullterm_scan<testing::AttributeCreateTraits>(file);
}

TEST_CASE("HighFiveSTDString (dataset, nullterm cornercase)") {
    auto file = HighFive::File("not_null_terminated_dataset.h5", HighFive::File::Truncate);
    check_supposedly_nullterm_scan<testing::DataSetCreateTraits>(file);
}

TEST_CASE("HighFiveSTDString (dataset, single, short)") {
    File file("std_string_dataset_single_short.h5", File::Truncate);
    check_single_string<testing::DataSetCreateTraits>(file, 3);
}

TEST_CASE("HighFiveSTDString (attribute, single, short)") {
    File file("std_string_attribute_single_short.h5", File::Truncate);
    check_single_string<testing::AttributeCreateTraits>(file, 3);
}

TEST_CASE("HighFiveSTDString (dataset, single, long)") {
    File file("std_string_dataset_single_long.h5", File::Truncate);
    check_single_string<testing::DataSetCreateTraits>(file, 256);
}

TEST_CASE("HighFiveSTDString (attribute, single, long)") {
    File file("std_string_attribute_single_long.h5", File::Truncate);
    check_single_string<testing::AttributeCreateTraits>(file, 256);
}

TEST_CASE("HighFiveSTDString (dataset, multiple, short)") {
    File file("std_string_dataset_multiple_short.h5", File::Truncate);
    check_multiple_string<testing::DataSetCreateTraits>(file, 3);
}

TEST_CASE("HighFiveSTDString (attribute, multiple, short)") {
    File file("std_string_attribute_multiple_short.h5", File::Truncate);
    check_multiple_string<testing::AttributeCreateTraits>(file, 3);
}

TEST_CASE("HighFiveSTDString (dataset, multiple, long)") {
    File file("std_string_dataset_multiple_long.h5", File::Truncate);
    check_multiple_string<testing::DataSetCreateTraits>(file, 256);
}

TEST_CASE("HighFiveSTDString (attribute, multiple, long)") {
    File file("std_string_attribute_multiple_long.h5", File::Truncate);
    check_multiple_string<testing::AttributeCreateTraits>(file, 256);
}

TEST_CASE("HighFiveFixedString") {
    const std::string file_name("array_atomic_types.h5");
    const std::string group_1("group1");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    char raw_strings[][10] = {"abcd", "1234"};

    /// This will not compile - only char arrays - hits static_assert with a nice
    /// error
    // file.createDataSet<int[10]>(ds_name, DataSpace(2)));

    {  // But char should be fine
        auto ds = file.createDataSet<char[10]>("ds1", DataSpace(2));
        CHECK(ds.getDataType().getClass() == DataTypeClass::String);
        ds.write(raw_strings);
    }

    {  // char[] is, by default, int8
        auto ds2 = file.createDataSet("ds2", raw_strings);
        CHECK(ds2.getDataType().getClass() == DataTypeClass::Integer);
    }

    {  // String Truncate happens low-level if well setup
        auto ds3 = file.createDataSet<char[6]>("ds3", DataSpace::FromCharArrayStrings(raw_strings));
        ds3.write(raw_strings);
    }

    {  // Write as raw elements from pointer (with const)
        const char(*strings_fixed)[10] = raw_strings;
        // With a pointer we dont know how many strings -> manual DataSpace
        file.createDataSet<char[10]>("ds4", DataSpace(2)).write(strings_fixed);
    }


    {  // Cant convert flex-length to fixed-length
        const char* buffer[] = {"abcd", "1234"};
        SilenceHDF5 silencer;
        CHECK_THROWS_AS(file.createDataSet<char[10]>("ds5", DataSpace(2)).write(buffer),
                        HighFive::DataSetException);
    }

    {  // scalar char strings
        const char buffer[] = "abcd";
        file.createDataSet<char[10]>("ds6", DataSpace(1)).write(buffer);
    }

    {
        // Direct way of writing `std::string` as a fixed length
        // HDF5 string.

        std::string value = "foo";
        auto n_chars = value.size() + 1;

        auto datatype = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
        auto dataspace = DataSpace(1);

        auto ds = file.createDataSet("ds8", dataspace, datatype);
        ds.write_raw(value.data(), datatype);

        {
            // Due to missing non-const overload of `data()` until C++17 we'll
            // read into something else instead (don't forget the '\0').
            auto expected = std::vector<char>(n_chars, '!');
            ds.read_raw(expected.data(), datatype);

            CHECK(expected.size() == value.size() + 1);
            for (size_t i = 0; i < value.size(); ++i) {
                REQUIRE(expected[i] == value[i]);
            }
        }

#if HIGHFIVE_CXX_STD >= 17
        {
            auto expected = std::string(value.size(), '-');
            ds.read_raw(expected.data(), datatype);

            REQUIRE(expected == value);
        }
#endif
    }

    {
        size_t n_chars = 4;
        size_t n_strings = 2;

        std::vector<char> value(n_chars * n_strings, '!');

        auto datatype = FixedLengthStringType(n_chars, StringPadding::NullTerminated);
        auto dataspace = DataSpace(n_strings);

        auto ds = file.createDataSet("ds9", dataspace, datatype);
        ds.write_raw(value.data(), datatype);

        auto expected = std::vector<char>(value.size(), '-');
        ds.read_raw(expected.data(), datatype);

        CHECK(expected.size() == value.size());
        for (size_t i = 0; i < value.size(); ++i) {
            REQUIRE(expected[i] == value[i]);
        }
    }
}
