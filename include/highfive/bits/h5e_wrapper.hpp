#pragma once

#include <H5Epublic.h>
namespace HighFive {
namespace detail {
namespace nothrow {


inline void h5e_get_auto2(hid_t estack_id, H5E_auto2_t* func, void** client_data) {
    H5Eget_auto2(estack_id, func, client_data);
}

inline void h5e_set_auto2(hid_t estack_id, H5E_auto2_t func, void* client_data) {
    H5Eset_auto2(estack_id, func, client_data);
}

inline char* h5e_get_major(H5E_major_t maj) {
    return H5Eget_major(maj);
}

inline char* h5e_get_minor(H5E_minor_t min) {
    return H5Eget_minor(min);
}

inline herr_t h5e_walk2(hid_t err_stack,
                        H5E_direction_t direction,
                        H5E_walk2_t func,
                        void* client_data) {
    return H5Ewalk2(err_stack, direction, func, client_data);
}

inline herr_t h5e_clear2(hid_t err_stack) {
    return H5Eclear2(err_stack);
}


}  // namespace nothrow
}  // namespace detail
}  // namespace HighFive
