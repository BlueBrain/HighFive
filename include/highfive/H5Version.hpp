/*
 *  Copyright (c), 2020
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#define HIGHFIVE_VERSION_MAJOR 3
#define HIGHFIVE_VERSION_MINOR 0
#define HIGHFIVE_VERSION_PATCH 0

// Undefined for regular releases.
#define HIGHFIVE_VERSION_PRERELEASE 1

/** \brief Concatenated representation of the HighFive version.
 *
 *  \warning The macro `HIGHFIVE_VERSION` by itself isn't valid C/C++.
 *
 *  However, it can be stringified with two layers of macros, e.g.,
 *  \code{.cpp}
 *  #define STRINGIFY_VALUE(s) STRINGIFY_NAME(s)
 *  #define STRINGIFY_NAME(s) #s
 *
 *  std::cout << STRINGIFY_VALUE(HIGHFIVE_VERSION) << "\n";
 *  \endcode
 */
#define HIGHFIVE_VERSION 3.0.0

/** \brief String representation of the HighFive version.
 *
 *  \warning This macro only exists from 2.7.1 onwards.
 */
#define HIGHFIVE_VERSION_STRING "3.0.0-beta1"
