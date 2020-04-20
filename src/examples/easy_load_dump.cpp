/// To enable plug-ins, load the relevant libraries BEFORE HighFive. E.g.
///
///   #include <xtensor/xtensor.hpp>
///   #include <Eigen/Eigen>
///   #include <highfive/H5Easy.hpp>
///
/// or ask HighFive to include them. E.g.
///
///   #define H5_USE_XTENSOR
///   #define H5_USE_EIGEN
///   #include <highfive/H5Easy.hpp>
///

// optionally enable plug-in xtensor
#ifdef H5_USE_XTENSOR
#include <xtensor/xtensor.hpp>
#endif

// optionally enable plug-in Eigen
#ifdef H5_USE_EIGEN
#include <Eigen/Eigen>
#endif

#include <highfive/H5Easy.hpp>

int main()
{
    H5Easy::File file("example.h5", H5Easy::File::Overwrite);

    // (over)write and read scalar
    {
        int A = 10;

        H5Easy::dump(file, "/path/to/A", A);
        H5Easy::dump(file, "/path/to/A", A, H5Easy::DumpMode::Overwrite);
    }

    // (over)write and read std::vector
    {
        std::vector<double> B = {1., 2., 3.};

        H5Easy::dump(file, "/path/to/B", B);
        H5Easy::dump(file, "/path/to/B", B, H5Easy::DumpMode::Overwrite);

        B = H5Easy::load<std::vector<double>>(file, "/path/to/B");
    }

    // (over)write scalar in (automatically expanding) extendible DataSet,
    // read item from the DataSet
    {
        int C = 10;

        H5Easy::dump(file, "/path/to/C", C, {0});
        H5Easy::dump(file, "/path/to/C", C, {1});
        H5Easy::dump(file, "/path/to/C", C, {3});

        C = H5Easy::load<int>(file, "/path/to/C", {0});
    }

    // get the size/shape of a DataSet
    {
        // outputs "size_t"
        H5Easy::getSize(file, "/path/to/C");

        // outputs "std::vector<size_t>"
        H5Easy::getShape(file, "/path/to/C");
    }

#ifdef H5_USE_EIGEN
    // (over)write and read Eigen::Matrix
    {
        // matrix
        Eigen::MatrixXd D = Eigen::MatrixXd::Random(10,5);

        H5Easy::dump(file, "/path/to/D", D);
        H5Easy::dump(file, "/path/to/D", D, H5Easy::DumpMode::Overwrite);

        D = H5Easy::load<Eigen::MatrixXd>(file, "/path/to/D");


        Eigen::ArrayXd D2 = Eigen::ArrayXd::Random(30);

        H5Easy::dump(file, "/path/to/D2", D2);
        H5Easy::dump(file, "/path/to/D2", D2, H5Easy::DumpMode::Overwrite);

        D2 = H5Easy::load<Eigen::ArrayXd>(file, "/path/to/D2");
    }
#endif

#ifdef H5_USE_XTENSOR
    // (over)write and read xt::xtensor (or xt::xarray)
    {
        xt::xtensor<size_t,1> E = xt::arange<size_t>(10);

        H5Easy::dump(file, "/path/to/E", E);
        H5Easy::dump(file, "/path/to/E", E, H5Easy::DumpMode::Overwrite);

        E = H5Easy::load<xt::xtensor<size_t,1>>(file, "/path/to/E");
    }
#endif

    return 0;
}
