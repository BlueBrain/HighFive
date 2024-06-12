#include <hi5_dependent/read.hpp>

namespace hi5_dependent {
std::vector<double> read_vector(const HighFive::DataSet& dset) {
    return dset.read<std::vector<double>>();
}
}  // namespace hi5_dependent
