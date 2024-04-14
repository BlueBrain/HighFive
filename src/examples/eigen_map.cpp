#include <highfive/highfive.hpp>
#include <highfive/eigen.hpp>

// Example showing reading and writing of `Eigen::Map`. Using
// `Map<Matrix, ...>` as an example, but `Map<Array, ...>` works
// analogously.
//
// Both `Eigen::Matrix` and `Eigen::Vector` have their own examples.

int main() {
    HighFive::File file("eigen_map.h5", HighFive::File::Truncate);

    // Somehow allocate some memory:
    double* p1 = (double*) malloc(4 * 3 * sizeof(double));

    Eigen::Map<Eigen::MatrixXd> A(p1, 4, 3);

    // clang-format off
    A <<  1,  2,  3,
          4,  5,  6,
          7,  8,  9,
         10, 11, 12;
    // clang-format on
    std::cout << "A = \n" << A << "\n\n";

    // Write it to the file:
    file.createDataSet("mat", A);

    // ... and read it back as fixed-size and row-major:
    using Matrix43d = Eigen::Matrix<double, 4, 3, Eigen::RowMajor>;

    // Again, memory was obtain somehow, and we create an `Eigen::Map`
    // from it:
    double* p2 = (double*) malloc(4 * 3 * sizeof(double));
    Eigen::Map<Matrix43d> B(p2, 4, 3);

    // Since, we've pre-allocated the memory, we use the overload of `read`
    // accepts `B` and an argument. Note, this will throw if `B` needs to be
    // resized, because a map shouldn't resize the underlying memory:
    file.getDataSet("mat").read(B);

    std::cout << "B = \n" << B << "\n";

    free(p1);
    free(p2);

    return 0;
}
