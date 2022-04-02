#include <iostream>
#include <stdexcept>
#include <vector>
#include "hdf5.h"

#define NROWS 1000000  // 1M
#define ROW_LENGTH 10

const std::vector<std::vector<int>> data(NROWS, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

int main() {
    /* Create a new file using default properties. */
    hid_t file_id = H5Fcreate("dataset_integer_raw_improved.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /* Create the data space for the dataset. */
    hsize_t dims[] = {NROWS, ROW_LENGTH};
    hid_t dataspace_id = H5Screate_simple(2, dims, NULL);

    /* Create the dataset. */
    hid_t dataset_id =
        H5Dcreate2(file_id, "/dataset", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    herr_t status;

    /* It's faster to aggregate all rows in a single contiguous buffer to do less IO ops */
    std::vector<int> data_contig;
    data_contig.reserve(NROWS * ROW_LENGTH);
    for (const auto& row : data) {
        std::copy(row.begin(), row.end(), std::back_inserter(data_contig));
    }

    /* Write each row to the dataset. */

    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data_contig.data());
    status |= H5Sclose(dataspace_id);
    status |= H5Dclose(dataset_id);
    status |= H5Fclose(file_id);
    if (status != 0) {
        std::cerr << "Error while releasing resources" << std::endl;
    }
    return status;
}
