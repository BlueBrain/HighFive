#include "hdf5.h"
#include <iostream>
#include <stdexcept>
#include <vector>

#define NROWS      1000000  // 1M
#define ROW_LENGTH 10

const std::vector<std::vector<int>> data(NROWS, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

int do_iteration() {
    /* Create a new file using default properties. */
    hid_t file_id = H5Fcreate("dataset_integer_raw.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /* Create the data space for the dataset. */
    hsize_t dims[] = {NROWS, ROW_LENGTH};
    hid_t dataspace_id = H5Screate_simple(2, dims, NULL);

    // Row memspace
    hsize_t mem_dims[] = {1, ROW_LENGTH};
    hid_t memspace_id = H5Screate_simple(2, mem_dims, NULL);

    /* Create the dataset. */
    hid_t dataset_id = H5Dcreate2(
        file_id, "/dataset", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    herr_t status;

    /* Write each row to the dataset. */
    for (size_t i = 0; i < NROWS; i++) {
        // File Hyperslabs
        hsize_t count[] = {1, 10};
        hsize_t offset[] = {i, 0};
        status = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, offset, NULL, count, NULL);
        if (status != 0) {
            throw(std::runtime_error("H5Sselect_hyperslab failed"));
        }
        status = H5Dwrite(
            dataset_id, H5T_NATIVE_INT, memspace_id, dataspace_id, H5P_DEFAULT, data[i].data());
        if (status != 0) {
            throw(std::runtime_error("H5Dwrite failed"));
        }
    }

    status = H5Sclose(memspace_id);
    status |= H5Sclose(dataspace_id);
    status |= H5Dclose(dataset_id);
    status |= H5Fclose(file_id);
    if (status != 0) {
        std::cerr << "Error while releasing resources" << std::endl;
    }
    return status;
}

int main() {
    for (int i = 0; i < 200; i++) {
        do_iteration();
    }
}
