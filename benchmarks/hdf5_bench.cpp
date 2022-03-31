#include <stdexcept>
#include <vector>
#include "hdf5.h"

#define NROWS 1000000  // 1M
#define ROW_VALUES {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}


const std::vector<std::vector<int>> data(NROWS, ROW_VALUES);

int main() {
    hid_t   file_id, dataset_id, dataspace_id; /* identifiers */
    hsize_t dims[2];
    herr_t  status;

    /* Create a new file using default properties. */
    file_id = H5Fcreate("dataset_integer_raw.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    /* Create the data space for the dataset. */
    dims[0]      = NROWS;
    dims[1]      = data.size();
    dataspace_id = H5Screate_simple(2, dims, NULL);

    /* Create the dataset. */
    dataset_id =
        H5Dcreate2(file_id, "/dataset", H5T_STD_I32BE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* Write to the dataset. */
    for (const auto& row : data) {
        status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, row.data());
        if (status > 0) {
            throw(std::runtime_error("H5Dwrite failed"));
        }
    }

    /* End access to the dataset and release resources used by it. */
    status = H5Dclose(dataset_id);

    /* Terminate access to the data space. */
    status = H5Sclose(dataspace_id);

    /* Close the file. */
    status = H5Fclose(file_id);
}
