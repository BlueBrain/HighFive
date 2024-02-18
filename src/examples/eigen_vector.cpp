#include <highfive/highfive.hpp>
#include <highfive/eigen.hpp>

// Example showing reading and writing of `Eigen::Matrix`. Using
// `Eigen::Matrix` as an example, but `Eigen::Array` works analogously.
//
// Both `Eigen::Vector` and `Eigen::Map` have their own examples.

int main() {
    HighFive::File file("eigen_vector.h5", HighFive::File::Truncate);

    // Create a matrix.
    Eigen::VectorXd v(3);
    v << 1, 2, 3;
    std::cout << "v = \n" << v << "\n\n";

    // Write it to the file:
    file.createDataSet("col_vec", v);

    // The twist is that Eigen typedefs:
    //   using VectorXd = Matrix<double, Dynamic, 1>;
    //
    // Therefore, for HighFive it's indistinguishable from a Nx1 matrix. Since,
    // Eigen distinguishes row and column vectors, the HighFive chooses to
    // respect the distinction and deduces the shape of vector as Nx1.

    // ... and read it back as fixed-size:
    auto w = file.getDataSet("col_vec").read<Eigen::Vector3d>();

    std::cout << "w = \n" << w << "\n";

    return 0;
}
