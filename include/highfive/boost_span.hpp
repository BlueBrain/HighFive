#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"
#include "bits/inspector_stl_span_misc.hpp"

#include <boost/core/span.hpp>

namespace HighFive {
namespace details {
template <class T, std::size_t Extent>
struct inspector<boost::span<T, Extent>>: public inspector_stl_span<boost::span<T, Extent>> {
  private:
    using super = inspector_stl_span<boost::span<T, Extent>>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

}  // namespace details
}  // namespace HighFive
