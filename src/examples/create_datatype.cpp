
#include <iostream>

#include <highfive/H5File.hpp>
#include <highfive/H5DataType.hpp>

using namespace HighFive;

const std::string FILE_NAME("create_datatype_example.h5");
const std::string DATASET_NAME("test_dataset");


// Struct representation of custom type (type v below)
typedef struct {
    char a;
    short b;
    unsigned long long c;
} csl;


// Tell HighFive how to create the HDF5 datatype for this base type by
// specialising the create_datatype template
namespace HighFive {
template <> inline DataType create_datatype<csl>() {
    CompoundType v_aligned;
    v_aligned.addMember("u1", H5T_NATIVE_UCHAR);
    v_aligned.addMember("u2", H5T_NATIVE_SHORT);
    v_aligned.addMember("u3", H5T_NATIVE_ULLONG);
    v_aligned.autoCreate();

    return v_aligned;
}
}

int main(void) {

    try {

        File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate);

        // Create a simple compound type with automatic alignment of the
        // members. For this the type alignment is trivial.
        CompoundType t;
        t.addMember("real", H5T_NATIVE_INT);
        t.addMember("imag", H5T_NATIVE_INT);
        t.autoCreate();
        t.commit(file, "new_type1");

        // Create a complex nested datatype with manual alignment
        CompoundType u;
        u.addMember("u1", t, 0);
        u.addMember("u2", t, 9);
        u.addMember("u3", H5T_NATIVE_INT, 20);
        u.manualCreate(26);
        u.commit(file, "new_type3");

        // Create a more complex type with automatic alignment. For this the
        // type alignment is more complex.
        CompoundType v_aligned;
        v_aligned.addMember("u1", H5T_NATIVE_UCHAR);
        v_aligned.addMember("u2", H5T_NATIVE_SHORT);
        v_aligned.addMember("u3", H5T_NATIVE_ULLONG);
        v_aligned.autoCreate();
        v_aligned.commit(file, "new_type2_aligned");

        // Create a more complex type with a fully packed alignment. The
        // equivalent type is created with a standard struct alignment in the
        // implementation of HighFive::create_datatype above
        CompoundType v_packed;
        v_packed.addMember("u1", H5T_NATIVE_UCHAR, 0);
        v_packed.addMember("u2", H5T_NATIVE_SHORT, 1);
        v_packed.addMember("u3", H5T_NATIVE_ULLONG, 3);
        v_packed.manualCreate(11);
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

        for(const auto& el : result) {
            std::cout << "CSL:" << std::endl;
            std::cout << "  " << el.a << std::endl;
            std::cout << "  " << el.b << std::endl;
            std::cout << "  " << el.c << std::endl;
        }


    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0; // successfully terminated
}
