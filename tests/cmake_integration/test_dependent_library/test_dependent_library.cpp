#include <hi5_dependent/read.hpp>
#include <hi5_dependent/write.hpp>


int main() {
    {
        auto file = HighFive::File("foo.h5", HighFive::File::Truncate);

        auto dset = hi5_dependent::write_vector(file, {1.0, 2.0, 3.0});
        auto x = hi5_dependent::read_vector(dset);

        for (size_t i = 0; i < x.size(); i++) {
            if (x[i] != double(i + 1)) {
                throw std::runtime_error("HighFiveDemo is broken.");
            }
        }

        std::cout << "Hi5Dependent: success \n";
    }

#if HI5_DEPENDENT_HAS_BOOST == 1
    {
        auto file = HighFive::File("bar.h5", HighFive::File::Truncate);

        boost::numeric::ublas::matrix<double> x(3, 5);
        auto dset = hi5_dependent::write_boost(file, x);
        auto y = hi5_dependent::read_boost(dset);

        std::cout << "Hi5BoostDependent: success \n";
    }
#endif

    return 0;
}
