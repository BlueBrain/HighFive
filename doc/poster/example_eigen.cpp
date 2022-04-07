#include <iostream>

#define H5_USE_EIGEN 1

#include <Eigen/Dense>
#include <highfive/H5File.hpp>

using namespace HighFive;

void data_io(void) {
    const std::string DATASET_NAME("dset");
    const int nrows = 10;
    const int ncols = 3;

    try {
        Eigen::MatrixXd mat(nrows, ncols);

        for (int i = 0; i < nrows; ++i) {
            for (int j = 0; j < ncols; ++j) {
                mat(i, j) = double(j + i * 100);
            }
        }

        File file("eigen_mat.h5", File::ReadWrite | File::Create | File::Truncate);

        DataSet dset = file.createDataSet(DATASET_NAME, mat);
        dset.write(mat);

        Eigen::MatrixXd result;
        dset.read(result);

    } catch (const Exception& err) {
        std::cerr << err.what() << std::endl;
    }
}
