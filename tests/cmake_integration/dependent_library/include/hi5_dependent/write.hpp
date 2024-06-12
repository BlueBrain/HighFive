#pragma once

#include <highfive/highfive.hpp>
#include <vector>

namespace hi5_dependent {
HighFive::DataSet write_vector(HighFive::File& file, const std::vector<double>& x);
}
