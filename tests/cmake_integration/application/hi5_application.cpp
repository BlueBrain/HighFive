#include <highfive/highfive.hpp>
#if HI5_APPLICATION_HAS_BOOST == 1
#include <highfive/boost.hpp>
#endif

int main() {
    {
        auto file = HighFive::File("foo.h5", HighFive::File::Truncate);

        auto dset = file.createDataSet("foo", std::vector<double>{1.0, 2.0, 3.0});
        auto x = dset.read<std::vector<double>>();

        for (size_t i = 0; i < x.size(); i++) {
            if (x[i] != double(i + 1)) {
                throw std::runtime_error("HighFiveDemo is broken.");
            }
        }

        std::cout << "Hi5Application: success \n";
    }

#if HI5_APPLICATION_HAS_BOOST == 1
    {
        using matrix_t = boost::numeric::ublas::matrix<double>;

        auto file = HighFive::File("bar.h5", HighFive::File::Truncate);
        matrix_t x(3, 5);
        auto dset = file.createDataSet("foo", x);
        auto y = dset.read<matrix_t>();

        std::cout << "Hi5BoostApplication: success \n";
    }
#endif

    return 0;
}
