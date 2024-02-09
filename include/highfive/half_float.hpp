#pragma once

#include <half.hpp>

namespace HighFive {
using float16_t = half_float::half;

template <>
inline AtomicType<float16_t>::AtomicType() {
    _hid = detail::h5t_copy(H5T_NATIVE_FLOAT);
    // Sign position, exponent position, exponent size, mantissa position, mantissa size
    detail::h5t_set_fields(_hid, 15, 10, 5, 0, 10);
    // Total datatype size (in bytes)
    detail::h5t_set_size(_hid, 2);
    // Floating point exponent bias
    detail::h5t_set_ebias(_hid, 15);
}

}  // namespace HighFive
