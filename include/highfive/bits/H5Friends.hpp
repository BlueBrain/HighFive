#pragma once

#ifndef HIGHFIVE_HAS_FRIEND_DECLARATIONS
#ifdef _MSC_VER
// This prevents a compiler bug on certain versions of MSVC.
// Known to fail: Toolset 141.
// See `CMakeLists.txt` for more information.
#define HIGHFIVE_HAS_FRIEND_DECLARATIONS 1
#endif
#endif