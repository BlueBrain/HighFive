#include <numeric>
#include <vector>

#include <highfive/highfive.hpp>

int main() {
    std::vector<double> large_attr(16000, 0.0);

    auto fapl = HighFive::FileAccessProps::Default();
    fapl.add(HighFive::FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    HighFive::File file("create_large_attribute.h5", HighFive::File::Truncate, fapl);
    auto gcpl = HighFive::GroupCreateProps::Default();
    gcpl.add(HighFive::AttributePhaseChange(0, 0));

    auto group = file.createGroup("grp", gcpl);
    group.createAttribute("attr", large_attr);

    return 0;
}
