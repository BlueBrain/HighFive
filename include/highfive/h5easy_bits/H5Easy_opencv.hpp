/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_OPENCV_HPP
#define H5EASY_BITS_OPENCV_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"  // to get the basic "load_impl"

////#ifdef H5_USE_OPENCV

namespace H5Easy {

namespace detail {

namespace opencv {

// return the shape of the opencv-object as "std::vector<size_t>"
template <class T>
inline std::vector<size_t> shape(const T& data)
{
    return std::vector<size_t>(data.shape().cbegin(), data.shape().cend());
}


// load opencv-object from DataSet
template <class T>
struct load_impl
{
    static T run(const File& file, const std::string& path)
    {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        T data(dims[0], dims[1]);
        dataset.read(reinterpret_cast<double*>(data.data));
        //                             ^ ! Shouldn't be explicitly double of course TODO.
        return data;
    }
};

}  // namespace opencv

// front-end
template <class T>
struct load_impl<cv::Mat_<T>>
{
    static cv::Mat_<T> run(const File& file, const std::string& path)
    {
        return detail::opencv::load_impl<cv::Mat_<T>>::run(file, path);
    }
};


}  // namespace detail

}  // namespace H5Easy

////////#endif  // H5_USE_OPENCV

#endif  // H5EASY_BITS_OPENCV_HPP
