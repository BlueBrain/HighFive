#pragma once

#include "compute_total_size.hpp"

namespace HighFive {

template <typename T>
using unqualified_t = typename std::remove_const<typename std::remove_reference<T>::type>::type;

namespace details {

template <typename T>
struct type_helper;

template <typename T>
struct inspector;

}  // namespace details
}  // namespace HighFive
