/*
 *  Copyright (c), 2023, Blue Brain Project, EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <string>
#include <vector>

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>

using namespace HighFive;

// This example shows how to write (containers of) `std::string`
// to dataset either as fixed or variable length HDF5 strings.
// The feature is available from 2.8.0 onwards.
int main(void) {
    auto file = File("read_write_std_string.h5", File::Truncate);

    // A string of length 3 in a buffer of size 4 bytes. We'll use "length" for
    // the semantic length of the string, i.e. excluding the '\0' character and
    // "size" to refer to the length of the buffer in which the string is stored.
    // For null-terminated strings, the `size == length + 1`.
    std::string ascii_string = "foo";
    auto scalar_dataspace = DataSpace(DataSpace::dataspace_scalar);

    // Just write the string:
    file.createDataSet("single_automatic", ascii_string);

    // The above results in writing the string as an HDF5 variable length UTF8
    // string. In HDF5 a variable length string doesn't specify the length of
    // the string. Variable length strings are always null-terminated.
    auto variable_stringtype = VariableLengthStringType();
    file.createDataSet("single_variable", scalar_dataspace, variable_stringtype)
        .write(ascii_string);

    // HDF5 also has the concept of fixed length string. In fixed length strings
    // the size of the string, in bytes, is part of the datatype. The HDF5 API
    // for fixed and variable length strings is distinct. Hence, when writing
    // string that need to be read by other programs, it can matter if the string
    // is stored as fixed or variable length.
    //
    // Important: The HDF5 string size is the size of the buffer required to
    // store the string.
    //
    // We know that ascii_string requires 4 bytes to store, but want to store
    // it in fixed length strings of length 8. Additionally, we promise that
    // the strings are null-terminated. The character set defaults to ASCII.
    auto fixed_stringtype = FixedLengthStringType(8, StringPadding::NullTerminated);
    file.createDataSet("single_fixed_nullterm", scalar_dataspace, fixed_stringtype)
        .write(ascii_string);

    // When reading into an `std::string` it doesn't matter if the HDF5 datatype
    // is fixed or variable length. HighFive will internally read into a buffer
    // and then write to the final destination.
    auto from_variable = file.getDataSet("single_variable").read<std::string>();
    auto from_fixed = file.getDataSet("single_fixed_nullterm").read<std::string>();

    // Note that because the fixed length string is null-terminated,
    // `from_fixed.size() == ascii_string.size()` despite it being stored as a string of
    // length 8.
    std::cout << "from_variable = '" << from_variable << "' size = " << from_variable.size()
              << "\n";
    std::cout << "from_fixed = '" << from_fixed << "' size = " << from_fixed.size() << "\n";

    // Fixed-length string don't have to be null-terminated. Their length could
    // be defined simply by the known size of the buffer required to store the
    // string. To deal with the situation where the string is shorter than the
    // buffer, one defines a padding character. This must be either the null or
    // space character. We'll show null-padded, space-padded works the same way.
    auto fixed_nullpad = FixedLengthStringType(8, StringPadding::NullPadded);
    file.createDataSet("single_fixed_nullpad", scalar_dataspace, fixed_nullpad).write(ascii_string);

    // Note that because we only know that the string is padded with nulls, but we
    // don't know if those nulls were part of the string to begin with. The full
    // size of the buffer is read into the `std::string`. The length of the
    // `std::string` is the size of the string type.
    auto from_nullpad = file.getDataSet("single_fixed_nullpad").read<std::string>();
    std::cout << "from_nullpad = '" << from_nullpad << "' size = " << from_nullpad.size() << "\n";

    // Let's look at UTF8 strings. In HDF5 the size of a string is the size in
    // bytes of the buffer required to store the string. A UTF8 symbol/character
    // requires 1 to 4 bytes.
    //
    // The 'a' is 1 byte, the 'α' 2 bytes, therefore a total of 3 bytes (same
    // as `utf8_string.size()`). Which including the null character fits into
    // 8 bytes. However, 8 bytes would, in general not be enough to store 2
    // UTF8 characters and the null character. Which would require 9 bytes.
    std::string utf8_string = "aα";
    auto fixed_utf8_type =
        FixedLengthStringType(8, StringPadding::NullTerminated, CharacterSet::Utf8);
    file.createDataSet("single_fixed_utf8", scalar_dataspace, fixed_utf8_type).write(utf8_string);

    auto from_utf8 = file.getDataSet("single_fixed_utf8").read<std::string>();
    std::cout << "from_utf8 = '" << from_utf8 << "' size = " << from_utf8.size() << "\n";

    // Finally, containers of `std::string`s work analogously:
    auto ascii_strings = std::vector<std::string>{"123", "456"};
    file.createDataSet("multi_fixed_nullterm", DataSpace::From(ascii_strings), fixed_stringtype)
        .write(ascii_strings);

    auto ascii_strings_from_fixed =
        file.getDataSet("multi_fixed_nullterm").read<std::vector<std::string>>();

    // In order to see details of how each is stored in the HDF5 file use:
    //     h5dump read_write_std_string.h5

    return 0;
}
