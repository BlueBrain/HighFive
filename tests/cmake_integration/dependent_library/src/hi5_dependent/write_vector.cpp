#include <hi5_dependent/write.hpp>

namespace hi5_dependent {

HighFive::DataSet write_vector(HighFive::File& file, const std::vector<double>& x) {
    return file.createDataSet("foo", x);
}

}  // namespace hi5_dependent
