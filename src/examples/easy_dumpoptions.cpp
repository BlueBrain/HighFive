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

    // plain options
    {
        std::vector<double> A = {1.0, 2.0, 3.0};

        H5Easy::dump(file, "/path/to/A", A);
        H5Easy::dump(file, "/path/to/A", A, H5Easy::DumpMode::Overwrite);
    }

    // advanced - compression
    {
        std::vector<double> B = {1.0, 2.0, 3.0};

        H5Easy::dump(file, "/path/to/B", B,
            H5Easy::DumpOptions(H5Easy::Compression()));

        H5Easy::dump(file, "/path/to/B", B,
            H5Easy::DumpOptions(H5Easy::Compression(), H5Easy::DumpMode::Overwrite));
    }

    // advanced - compression - set compression level
    {
        std::vector<double> C = {1.0, 2.0, 3.0};

        H5Easy::dump(file, "/path/to/C", C,
            H5Easy::DumpOptions(H5Easy::Compression(8)));
    }

    // advanced - compression - set compression level & chunk size
    {
        std::vector<double> D = {1.0, 2.0, 3.0};

        H5Easy::DumpOptions options(H5Easy::Compression(8));
        options.setChunkSize({3});

        H5Easy::dump(file, "/path/to/D", D, options);
    }

    // advanced - set chunk size
    {
        int E = 10;

        H5Easy::DumpOptions options;
        options.setChunkSize({100, 100});

        H5Easy::dump(file, "/path/to/E", E, {0, 0}, options);
        H5Easy::dump(file, "/path/to/E", E, {0, 1}, options);
        // ...
    }

    // advanced - no automatic flushing
    {
        std::vector<double> F = {1.0, 2.0, 3.0};

        H5Easy::dump(file, "/path/to/F", F,
            H5Easy::DumpOptions(H5Easy::Flush::False));

        file.flush();
    }

    return 0;
}
