#pragma once
#include <H5public.h>

namespace HighFive {
namespace detail {
inline void h5_free_memory(void* mem) {
    if (H5free_memory(mem) < 0) {
        throw DataTypeException("Could not free memory allocated by HDF5");
    }
}

namespace nothrow {
inline herr_t h5_free_memory(void* mem) {
    return H5free_memory(mem);
}
}  // namespace nothrow

}  // namespace detail
}  // namespace HighFive
