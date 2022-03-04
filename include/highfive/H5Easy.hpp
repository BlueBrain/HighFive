/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

/// \brief
/// Read/dump DataSets or Attribute using a minimalistic syntax.
/// To this end, the functions are templated, and accept:
/// - Any type accepted by HighFive
/// - Eigen objects
/// - xtensor objects
/// - OpenCV objects

#ifndef H5EASY_HPP
#define H5EASY_HPP

// The API can be found here:
#include "h5easy_bits/H5Easy_decl.hpp"

#include "h5easy_bits/H5Easy_public.hpp"
#include "h5easy_bits/H5Easy_Eigen.hpp"
#include "h5easy_bits/H5Easy_opencv.hpp"
#include "h5easy_bits/H5Easy_scalar.hpp"
#include "h5easy_bits/H5Easy_vector.hpp"
#include "h5easy_bits/H5Easy_xtensor.hpp"

#endif  // H5EASY_HPP
