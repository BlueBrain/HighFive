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

template<typename _Tp, template<typename> class T>
struct load_impl
{
    static T<_Tp> run(const File& file, const std::string& path)
    {
        DataSet dataset = file.getDataSet(path);
        std::vector<size_t> dims = dataset.getDimensions();
        T<_Tp> data(dims[0], dims[1]);
        dataset.read(reinterpret_cast<_Tp*>(data.data)); // idem v, choose
        //dataset.read(data.template ptr<_Tp>()); // idem ^, choose
        return data;
    }
};




}  // namespace opencv

// front-end
template <class _Tp>
struct load_impl<cv::Mat_<_Tp>>
{
    static cv::Mat_<_Tp> run(const File& file, const std::string& path)
    {
        return detail::opencv::load_impl<_Tp, cv::Mat_>::run(file, path);
    }
};


}  // namespace detail

}  // namespace H5Easy

////////#endif  // H5_USE_OPENCV

#endif  // H5EASY_BITS_OPENCV_HPP
