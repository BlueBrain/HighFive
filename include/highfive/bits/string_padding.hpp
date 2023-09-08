#pragma once

#include <H5Tpublic.h>

namespace HighFive {

enum class StringPadding : std::underlying_type<H5T_str_t>::type {
    NullTerminated = H5T_STR_NULLTERM,
    NullPadded = H5T_STR_NULLPAD,
    SpacePadded = H5T_STR_SPACEPAD
};


}
