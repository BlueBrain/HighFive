/// To enable plug-ins, load the relevant libraries BEFORE HighFive. E.g.
///
///   #include <xtensor/xtensor.hpp>
///   #include <Eigen/Eigen>
///   #include <highfive/H5Easy.hpp>
///
/// or ask HighFive to include them. E.g.
///
///   #define HIGHFIVE_XTENSOR
///   #define HIGHFIVE_EIGEN
///   #include <highfive/H5Easy.hpp>
///

// optionally enable plug-in xtensor
#ifdef HIGHFIVE_XTENSOR
#include <xtensor/xtensor.hpp>
#endif

// optionally enable plug-in Eigen
#ifdef HIGHFIVE_EIGEN
#include <Eigen/Eigen>
#endif

#include <highfive/H5Easy.hpp>

int main()
{
    HighFive::File file("example.h5", HighFive::File::Overwrite);

    // (over)write scalar
    {
        int A = 10;

        HighFive::dump(file, "/path/to/A", A);
        HighFive::dump(file, "/path/to/A", A, HighFive::Mode::Overwrite);
    }

    // (over)write std::vector
    {
        std::vector<double> B = {1., 2., 3.};

        HighFive::dump(file, "/path/to/B", B);
        HighFive::dump(file, "/path/to/B", B, HighFive::Mode::Overwrite);
    }

    // (over)write scalar in (automatically expanding) extendible DataSet
    {
        HighFive::dump(file, "/path/to/C", 10, {0});
        HighFive::dump(file, "/path/to/C", 11, {1});
        HighFive::dump(file, "/path/to/C", 12, {3});
    }

        // read scalar
    {
        int A = HighFive::load<int>(file, "/path/to/A");
    }

    // read std::vector
    {
        std::vector<double> B = HighFive::load<std::vector<double>>(file, "/path/to/B");
    }

    // read scalar from DataSet
    {
        int C = HighFive::load<int>(file, "/path/to/C", {0});
    }

    // get the size/shape of a DataSet
    {
        size_t size = HighFive::getSize(file, "/path/to/C");
        std::vector<size_t> shape = HighFive::getShape(file, "/path/to/C");
    }

#ifdef HIGHFIVE_EIGEN
    // (over)write Eigen::Matrix
    {
        Eigen::MatrixXd D = Eigen::MatrixXd::Random(10,5);

        HighFive::dump(file, "/path/to/D", D);
        HighFive::dump(file, "/path/to/D", D, HighFive::Mode::Overwrite);
    }
    // read Eigen::Matrix
    {
        Eigen::MatrixXd D = HighFive::load<Eigen::MatrixXd>(file, "/path/to/D");
    }
#endif

#ifdef HIGHFIVE_XTENSOR
    // (over)write xt::xtensor (or xt::xarray)
    {
        xt::xtensor<size_t,1> E = xt::arange<size_t>(10);

        HighFive::dump(file, "/path/to/E", E);
        HighFive::dump(file, "/path/to/E", E, HighFive::Mode::Overwrite);
    }
    // read xt::xtensor (or xt::xarray)
    {
        xt::xtensor<size_t,1> E = HighFive::load<xt::xtensor<size_t,1>>(file, "/path/to/E");
    }
#endif

    return 0;
}
