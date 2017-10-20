
#include <iostream>

#include <highfive/H5File.hpp>
#include <highfive/H5DataType.hpp>

using namespace HighFive;

const std::string FILE_NAME("create_datatype_example.h5");

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

        // Create a more complex type with automatic alignment. For this the
        // type alignment is more complex.
        CompoundType u;
        u.addMember("u1", H5T_NATIVE_UCHAR);
        u.addMember("u2", H5T_NATIVE_SHORT);
        u.addMember("u3", H5T_NATIVE_ULLONG);
        u.autoCreate();
        u.commit(file, "new_type2");

        // Create a complex nested datatype with manual alignment
        CompoundType v;
        v.addMember("v1", t, 0);
        v.addMember("v2", t, 9);
        v.addMember("v3", H5T_NATIVE_INT, 20);
        v.manualCreate(26);
        v.commit(file, "new_type3");

    } catch (Exception& err) {
        // catch and print any HDF5 error
        std::cerr << err.what() << std::endl;
    }

    return 0; // successfully terminated
}
