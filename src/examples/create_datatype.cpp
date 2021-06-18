#include <iostream>

#include <highfive/H5File.hpp>
#include <highfive/H5DataType.hpp>

using namespace HighFive;

static const std::string FILE_NAME("create_datatype_example.h5");
static const std::string DATASET_NAME("test_dataset");


// Struct representation of custom type (type v below)
typedef struct {
    char a;
    short b;
    unsigned long long c;
} csl;

bool operator==(csl x, csl y) {
    return x.a == y.a && x.b == y.b && x.c == y.c;
}

bool operator!=(csl x, csl y) {
    return !(x == y);
}

// Tell HighFive how to create the HDF5 datatype for this base type by
// using the HIGHFIVE_REGISTER_TYPE macro
CompoundType create_compound_csl() {
    return {{"u1", AtomicType<unsigned char>{}},
            {"u2", AtomicType<short>{}},
            {"u3", AtomicType<unsigned long long>{}}};
}
HIGHFIVE_REGISTER_TYPE(csl, create_compound_csl)

int main(void) {

    try {

        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create a simple compound type with automatic alignment of the
        // members. For this the type alignment is trivial.
        std::vector<CompoundType::member_def> t_members({
            {"real", AtomicType<int>{}},
            {"imag", AtomicType<int>{}}
        });
        CompoundType t(t_members);
        t.commit(file, "new_type1");

        // Create a complex nested datatype with manual alignment
        CompoundType u({{"u1", t, 0},
                        {"u2", t, 9},
                        {"u3", AtomicType<int>{}, 20}},
                       26);
        u.commit(file, "new_type3");

        // Create a more complex type with automatic alignment. For this the
        // type alignment is more complex.
        CompoundType v_aligned{{"u1", AtomicType<unsigned char>{}},
                               {"u2", AtomicType<short>{}},
                               {"u3", AtomicType<unsigned long long>{}}};
        // introspect the compound type
        std::cout << "v_aligned size: " << v_aligned.getSize();
        for (const auto& member : v_aligned.getMembers()) {
            std::cout << "  field " << member.name << " offset: " << member.offset
                      << std::endl;
        }

        v_aligned.commit(file, "new_type2_aligned");

        // Create a more complex type with a fully packed alignment. The
        // equivalent type is created with a standard struct alignment in the
        // implementation of HighFive::create_datatype above
        CompoundType v_packed({{"u1", AtomicType<unsigned char>{}, 0},
                               {"u2", AtomicType<short>{}, 1},
                               {"u3", AtomicType<unsigned long long>{}, 3}},
                              11);
        v_packed.commit(file, "new_type2_packed");


        // Initialise some data
        std::vector<csl> data;
        data.push_back({'f', 1, 4});
        data.push_back({'g', -4, 18});

        // Write the data into the file in a fully packed form
        DataSet dataset = file.createDataSet(DATASET_NAME, DataSpace(2), v_packed);
        dataset.write(data);

        file.flush();

        // Read a subset of the data back
        std::vector<csl> result;
        dataset.select({0}, {2}).read(result);

        for(size_t i = 0; i < data.size(); ++i) {
            if (result[i] != data[i]) {
                std::cout << "result[" << i << "]:" << std::endl;
                std::cout << "    " << result[i].a << std::endl;
                std::cout << "    " << result[i].b << std::endl;
                std::cout << "    " << result[i].c << std::endl;
                std::cout << "data[" << i << "]:" << std::endl;
                std::cout << "    " << data[i].a << std::endl;
                std::cout << "    " << data[i].b << std::endl;
                std::cout << "    " << data[i].c << std::endl;
            }
        }


    } catch (const Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
        return 1;
    }

    return 0; // successfully terminated
}
