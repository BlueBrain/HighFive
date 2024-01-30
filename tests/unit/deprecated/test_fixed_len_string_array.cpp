#include <catch2/catch_test_macros.hpp>

#include <highfive/highfive.hpp>
#include "../tests_high_five.hpp"

namespace HighFive {

TEST_CASE("HighFiveFixedLenStringArray") {
    const std::string file_name("fixed_len_string_array.h5");

    // Create a new file using the default property lists.
    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    {  // Dedicated FixedLenStringArray (now deprecated).
        FixedLenStringArray<10> arr{"0000000", "1111111"};

        // More API: test inserting something
        arr.push_back("2222");
        auto ds = file.createDataSet("ds7", arr);  // Short syntax ok

        // Recover truncating
        FixedLenStringArray<4> array_back;
        ds.read(array_back);
        CHECK(array_back.size() == 3);
        CHECK(array_back[0] == std::string("000"));
        CHECK(array_back[1] == std::string("111"));
        CHECK(array_back[2] == std::string("222"));
        CHECK(array_back.getString(1) == "111");
        CHECK(array_back.front() == std::string("000"));
        CHECK(array_back.back() == std::string("222"));
        CHECK(array_back.data() == std::string("000"));
        array_back.data()[0] = 'x';
        CHECK(array_back.data() == std::string("x00"));

        for (auto& raw_elem: array_back) {
            raw_elem[1] = 'y';
        }
        CHECK(array_back.getString(1) == "1y1");
        for (auto iter = array_back.cbegin(); iter != array_back.cend(); ++iter) {
            CHECK((*iter)[1] == 'y');
        }
    }
}

template <size_t N>
static void check_fixed_len_string_array_contents(const FixedLenStringArray<N>& array,
                                                  const std::vector<std::string>& expected) {
    REQUIRE(array.size() == expected.size());

    for (size_t i = 0; i < array.size(); ++i) {
        CHECK(array[i] == expected[i]);
    }
}


TEST_CASE("HighFiveFixedLenStringArrayStructure") {
    using fixed_array_t = FixedLenStringArray<10>;
    // increment the characters of a string written in a std::array
    auto increment_string = [](const fixed_array_t::value_type arr) {
        fixed_array_t::value_type output(arr);
        for (auto& c: output) {
            if (c == 0) {
                break;
            }
            ++c;
        }
        return output;
    };

    SECTION("create from std::vector (onpoint)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<4>(expected);
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from std::vector (oversized)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<8>(expected);
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from pointers (onpoint)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<4>(expected.data(), expected.data() + expected.size());
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from pointers (oversized)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<8>(expected.data(), expected.data() + expected.size());
        check_fixed_len_string_array_contents(actual, expected);
    }


    SECTION("create from std::initializer_list (onpoint)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<4>{"000", "111"};
        check_fixed_len_string_array_contents(actual, expected);
    }

    SECTION("create from std::initializer_list (oversized)") {
        auto expected = std::vector<std::string>{"000", "111"};
        auto actual = FixedLenStringArray<8>{"000", "111"};
        check_fixed_len_string_array_contents(actual, expected);
    }

    // manipulate FixedLenStringArray with std::copy
    SECTION("compatible with std::copy") {
        const fixed_array_t arr1{"0000000", "1111111"};
        fixed_array_t arr2{"0000000", "1111111"};
        std::copy(arr1.begin(), arr1.end(), std::back_inserter(arr2));
        CHECK(arr2.size() == 4);
    }

    SECTION("compatible with std::transform") {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::transform(arr1.begin(), arr1.end(), std::back_inserter(arr), increment_string);
        }
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("1111111"));
        CHECK(arr[1] == std::string("2222222"));
    }

    SECTION("compatible with std::transform (reverse iterator)") {
        fixed_array_t arr;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::copy(arr1.rbegin(), arr1.rend(), std::back_inserter(arr));
        }
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("1111111"));
        CHECK(arr[1] == std::string("0000000"));
    }

    SECTION("compatible with std::remove_copy_if") {
        fixed_array_t arr2;
        {
            const fixed_array_t arr1{"0000000", "1111111"};
            std::remove_copy_if(arr1.begin(),
                                arr1.end(),
                                std::back_inserter(arr2),
                                [](const fixed_array_t::value_type& s) {
                                    return std::strncmp(s.data(), "1111111", 7) == 0;
                                });
        }
        CHECK(arr2.size() == 1);
        CHECK(arr2[0] == std::string("0000000"));
    }
}

TEST_CASE("HighFiveFixedLenStringArrayAttribute") {
    const std::string file_name("fixed_array_attr.h5");
    // Create a new file using the default property lists.
    {
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);
        FixedLenStringArray<10> arr{"Hello", "world"};
        file.createAttribute("str", arr);
    }
    // Re-read it
    {
        File file(file_name);
        FixedLenStringArray<8> arr;  // notice the output strings can be smaller
        file.getAttribute("str").read(arr);
        CHECK(arr.size() == 2);
        CHECK(arr[0] == std::string("Hello"));
        CHECK(arr[1] == std::string("world"));
    }
}

}  // namespace HighFive
