/*
 *  Copyright (c), 2017-2023, Blue Brain Project - EPFL
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>


#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <highfive/highfive.hpp>
#include "tests_high_five.hpp"

using namespace HighFive;
using Catch::Matchers::Equals;

TEST_CASE("Simple test for type equality") {
    AtomicType<double> d_var;
    AtomicType<size_t> size_var;
    AtomicType<double> d_var_test;
    AtomicType<size_t> size_var_cpy(size_var);
    AtomicType<int> int_var;
    AtomicType<unsigned> uint_var;

    // check different type matching
    CHECK(d_var == d_var_test);
    CHECK(d_var != size_var);

    // check type copy matching
    CHECK(size_var_cpy == size_var);

    // check sign change not matching
    CHECK(int_var != uint_var);
}


struct CSL1 {
    int m1;
    int m2;
    int m3;
};

struct CSL2 {
    CSL1 csl1;
};

CompoundType create_compound_csl1() {
    auto t2 = AtomicType<int>();
    CompoundType t1({{"m1", AtomicType<int>{}}, {"m2", AtomicType<int>{}}, {"m3", t2}});

    return t1;
}

CompoundType create_compound_csl2() {
    CompoundType t1 = create_compound_csl1();

    CompoundType t2({{"csl1", t1}});

    return t2;
}

HIGHFIVE_REGISTER_TYPE(CSL1, create_compound_csl1)
HIGHFIVE_REGISTER_TYPE(CSL2, create_compound_csl2)

TEST_CASE("HighFiveCompounds") {
    const std::string file_name("compounds_test.h5");
    const std::string dataset_name1("/a");
    const std::string dataset_name2("/b");

    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    auto t3 = AtomicType<int>();
    CompoundType t1 = create_compound_csl1();
    t1.commit(file, "my_type");

    CompoundType t2 = create_compound_csl2();
    t2.commit(file, "my_type2");

    {  // Not nested
        auto dataset = file.createDataSet(dataset_name1, DataSpace(2), t1);

        std::vector<CSL1> csl = {{1, 1, 1}, {2, 3, 4}};
        dataset.write(csl);

        file.flush();

        std::vector<CSL1> result;
        dataset.select({0}, {2}).read(result);

        CHECK(result.size() == 2);
        CHECK(result[0].m1 == 1);
        CHECK(result[0].m2 == 1);
        CHECK(result[0].m3 == 1);
        CHECK(result[1].m1 == 2);
        CHECK(result[1].m2 == 3);
        CHECK(result[1].m3 == 4);
    }

    {  // Nested
        auto dataset = file.createDataSet(dataset_name2, DataSpace(2), t2);

        std::vector<CSL2> csl = {{{1, 1, 1}, {2, 3, 4}}};
        dataset.write(csl);

        file.flush();
        std::vector<CSL2> result = {{{1, 1, 1}, {2, 3, 4}}};
        dataset.select({0}, {2}).read(result);

        CHECK(result.size() == 2);
        CHECK(result[0].csl1.m1 == 1);
        CHECK(result[0].csl1.m2 == 1);
        CHECK(result[0].csl1.m3 == 1);
        CHECK(result[1].csl1.m1 == 2);
        CHECK(result[1].csl1.m2 == 3);
        CHECK(result[1].csl1.m3 == 4);
    }

    // Test the constructor from hid
    CompoundType t1_from_hid(t1);
    CHECK(t1 == t1_from_hid);

    CompoundType t2_from_hid(t2);
    CHECK(t2 == t2_from_hid);

    // Back from a DataType
    CHECK_NOTHROW(CompoundType(DataType(t1_from_hid)));
    CHECK_THROWS(CompoundType(AtomicType<uint32_t>{}));
}

struct GrandChild {
    uint32_t gcm1;
    uint32_t gcm2;
    uint32_t gcm3;
};

struct Child {
    GrandChild grandChild;
    uint32_t cm1;
};

struct Parent {
    uint32_t pm1;
    Child child;
};

CompoundType create_compound_GrandChild() {
    auto t2 = AtomicType<uint32_t>();
    CompoundType t1({{"gcm1", AtomicType<uint32_t>{}},
                     {"gcm2", AtomicType<uint32_t>{}},
                     {
                         "gcm3",
                         t2,
                     }});
    return t1;
}

CompoundType create_compound_Child() {
    auto nestedType = create_compound_GrandChild();
    return CompoundType{{{
                             "grandChild",
                             nestedType,
                         },
                         {"cm1", AtomicType<uint32_t>{}}}};
}

CompoundType create_compound_Parent() {
    auto nestedType = create_compound_Child();
    return CompoundType{{{"pm1", AtomicType<uint32_t>{}},
                         {
                             "child",
                             nestedType,
                         }}};
}

HIGHFIVE_REGISTER_TYPE(GrandChild, create_compound_GrandChild)
HIGHFIVE_REGISTER_TYPE(Child, create_compound_Child)
HIGHFIVE_REGISTER_TYPE(Parent, create_compound_Parent)

TEST_CASE("HighFiveCompoundsNested") {
    const std::string file_name("nested_compounds_test.h5");
    const std::string dataset_name("/a");

    {  // Write
        File file(file_name, File::ReadWrite | File::Create | File::Truncate);
        auto type = create_compound_Parent();

        auto dataset = file.createDataSet(dataset_name, DataSpace(2), type);
        CHECK(dataset.getDataType().getSize() == 20);

        std::vector<Parent> csl = {Parent{1, Child{GrandChild{1, 1, 1}, 1}},
                                   Parent{2, Child{GrandChild{3, 4, 5}, 6}}};
        dataset.write(csl);
    }

    {  // Read
        File file(file_name, File::ReadOnly);
        std::vector<Parent> result;
        auto dataset = file.getDataSet(dataset_name);
        CHECK(dataset.getDataType().getSize() == 20);
        dataset.select({0}, {2}).read(result);

        CHECK(result.size() == 2);
        CHECK(result[0].pm1 == 1);
        CHECK(result[0].child.grandChild.gcm1 == 1);
        CHECK(result[0].child.grandChild.gcm2 == 1);
        CHECK(result[0].child.grandChild.gcm3 == 1);
        CHECK(result[0].child.cm1 == 1);
        CHECK(result[1].pm1 == 2);
        CHECK(result[1].child.grandChild.gcm1 == 3);
        CHECK(result[1].child.grandChild.gcm2 == 4);
        CHECK(result[1].child.grandChild.gcm3 == 5);
        CHECK(result[1].child.cm1 == 6);
    }
}

template <size_t N>
struct Record {
    double d = 3.14;
    int i = 42;
    char s[N];
};

template <size_t N>
void fill(Record<N>& r) {
    constexpr char ref[] = "123456789a123456789b123456789c123456789d123456789e123456789f";
    std::copy(ref, ref + N - 1, r.s);
    r.s[N - 1] = '\0';
}

template <size_t N>
CompoundType rec_t() {
    using RecN = Record<N>;
    return {{"d", create_datatype<decltype(RecN::d)>()},
            {"i", create_datatype<decltype(RecN::i)>()},
            {"s", create_datatype<decltype(RecN::s)>()}};
}

HIGHFIVE_REGISTER_TYPE(Record<4>, rec_t<4>)
HIGHFIVE_REGISTER_TYPE(Record<8>, rec_t<8>)
HIGHFIVE_REGISTER_TYPE(Record<9>, rec_t<9>)

template <size_t N>
void save(File& f) {
    const size_t numRec = 2;
    std::vector<Record<N>> recs(numRec);
    fill<N>(recs[0]);
    fill<N>(recs[1]);
    auto dataset = f.createDataSet<Record<N>>("records" + std::to_string(N), DataSpace::From(recs));
    dataset.write(recs);
}

template <size_t N>
std::string check(File& f) {
    const size_t numRec = 2;
    std::vector<Record<N>> recs(numRec);
    f.getDataSet("records" + std::to_string(N)).read(recs);
    return std::string(recs[0].s);
}

TEST_CASE("HighFiveCompoundsSeveralPadding") {
    const std::string file_name("padded_compounds_test.h5");

    File file(file_name, File::ReadWrite | File::Create | File::Truncate);
    {  // Write
        // 4 have been choose because no padding
        // /* offset      |    size */  type = struct Record<4> {
        // /*      0      |       8 */    double d;
        // /*      8      |       4 */    int i;
        // /*     12      |       4 */    char s[4];
        // total size (bytes):   16
        CHECK_NOTHROW(save<4>(file));
        // 8 have been choose because there is a padding
        // /* offset      |    size */  type = struct Record<8> {
        // /*      0      |       8 */    double d;
        // /*      8      |       4 */    int i;
        // /*     12      |       8 */    char s[8];
        // /* XXX  4-byte padding   */
        // total size (bytes):   24
        CHECK_NOTHROW(save<8>(file));
        // 9 have been choose because there should not be a padding on 9
        // /* offset      |    size */  type = struct Record<9> {
        // /*      0      |       8 */    double d;
        // /*      8      |       4 */    int i;
        // /*     12      |       9 */    char s[9];
        // /* XXX  3-byte padding   */
        // total size (bytes):   24
        CHECK_NOTHROW(save<9>(file));
    }

    {  // Read
        CHECK(check<4>(file) == std::string("123"));
        CHECK(check<8>(file) == std::string("1234567"));
        CHECK(check<9>(file) == std::string("12345678"));
    }
}

enum Position {
    highfive_first = 1,
    highfive_second = 2,
    highfive_third = 3,
    highfive_last = -1,
};

enum class Direction : signed char {
    Forward = 1,
    Backward = -1,
    Left = -2,
    Right = 2,
};

EnumType<Position> create_enum_position() {
    return {{"highfive_first", Position::highfive_first},
            {"highfive_second", Position::highfive_second},
            {"highfive_third", Position::highfive_third},
            {"highfive_last", Position::highfive_last}};
}
HIGHFIVE_REGISTER_TYPE(Position, create_enum_position)

EnumType<Direction> create_enum_direction() {
    return {{"Forward", Direction::Forward},
            {"Backward", Direction::Backward},
            {"Left", Direction::Left},
            {"Right", Direction::Right}};
}
HIGHFIVE_REGISTER_TYPE(Direction, create_enum_direction)

TEST_CASE("HighFiveEnum") {
    const std::string file_name("enum_test.h5");
    const std::string dataset_name1("/a");
    const std::string dataset_name2("/b");

    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    {  // Unscoped enum
        auto e1 = create_enum_position();
        e1.commit(file, "Position");

        auto dataset = file.createDataSet(dataset_name1, DataSpace(1), e1);
        dataset.write(Position::highfive_first);

        file.flush();

        Position result;
        dataset.select(ElementSet({0})).read(result);

        CHECK(result == Position::highfive_first);
    }

    {  // Scoped enum
        auto e1 = create_enum_direction();
        e1.commit(file, "Direction");

        auto dataset = file.createDataSet(dataset_name2, DataSpace(5), e1);
        std::vector<Direction> robot_moves({Direction::Backward,
                                            Direction::Forward,
                                            Direction::Forward,
                                            Direction::Left,
                                            Direction::Left});
        dataset.write(robot_moves);

        file.flush();

        std::vector<Direction> result;
        dataset.read(result);

        CHECK(result[0] == Direction::Backward);
        CHECK(result[1] == Direction::Forward);
        CHECK(result[2] == Direction::Forward);
        CHECK(result[3] == Direction::Left);
        CHECK(result[4] == Direction::Left);
    }
}

TEST_CASE("HighFiveReadType") {
    const std::string file_name("readtype_test.h5");
    const std::string datatype_name1("my_type");
    const std::string datatype_name2("position");

    File file(file_name, File::ReadWrite | File::Create | File::Truncate);

    CompoundType t1 = create_compound_csl1();
    t1.commit(file, datatype_name1);

    CompoundType t2 = file.getDataType(datatype_name1);

    auto t3 = create_enum_position();
    t3.commit(file, datatype_name2);

    DataType t4 = file.getDataType(datatype_name2);

    CHECK(t2 == t1);
    CHECK(t4 == t3);
}
