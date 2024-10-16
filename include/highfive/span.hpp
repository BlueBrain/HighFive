/*
 *  Copyright (c) 2024 Blue Brain Project
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "bits/inspector_stl_span_misc.hpp"

#include <span>

namespace HighFive {
namespace details {

template <class T, std::size_t Extent>
struct inspector<std::span<T, Extent>>: public inspector_stl_span<std::span<T, Extent>> {
  private:
    using super = inspector_stl_span<std::span<T, Extent>>;

  public:
    using type = typename super::type;
    using value_type = typename super::value_type;
    using base_type = typename super::base_type;
    using hdf5_type = typename super::hdf5_type;
};

}  // namespace details
}  // namespace HighFive
