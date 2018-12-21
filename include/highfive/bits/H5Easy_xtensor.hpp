/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_XTENSOR_HPP
#define H5EASY_BITS_XTENSOR_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"

#ifdef HIGHFIVE_XTENSOR

namespace HighFive
{
    namespace detail
    {
        namespace xtensor
        {
            template<class T>
            struct dump_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                {
                    createGroupsToDataSet(file, path);

                    std::vector<std::size_t> dims(data.shape().cbegin(), data.shape().cend());

                    HighFive::DataSet dataset = file.createDataSet<typename C::value_type>(path, HighFive::DataSpace(dims));

                    dataset.write(data.begin());

                    file.flush();

                    return dataset;
                }
            };

            template<class T>
            struct overwrite_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                {
                    HighFive::DataSet dataset = file.getDataSet(path);

                    auto dataspace = dataset.getSpace();

                    auto dims = dataspace.getDimensions();

                    if (data.shape().size() != dims.size())
                    {
                        throw detail::error(file, path, "HighFive::dump: Inconsistent rank");
                    }

                    for (std::size_t i = 0; i < data.shape().size(); ++i)
                    {
                        if (data.shape()[i] != dims[i])
                        {
                            throw detail::error(file, path, "HighFive::dump: Inconsistent dimensions");
                        }
                    }

                    dataset.write(data.begin());

                    file.flush();

                    return dataset;
                }
            };

            template <class T>
            struct load_impl
            {
                static auto run(const HighFive::File& file, const std::string& path)
                {
                    HighFive::DataSet dataset = file.getDataSet(path);

                    auto dataspace = dataset.getSpace();

                    auto dims = dataspace.getDimensions();

                    T data = T::from_shape(dims);

                    dataset.read(data.data());

                    return data;
                }
            };
        }

        template <class T>
        struct load_impl<xt::xarray<T>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::xtensor::load_impl<xt::xarray<T>>::run(file, path);
            }
        };

        template <class T, std::size_t rank>
        struct load_impl<xt::xtensor<T,rank>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::xtensor::load_impl<xt::xtensor<T,rank>>::run(file, path);
            }
        };
    }

    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xarray<T>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !file.exist(path)))
        {
            return detail::xtensor::dump_impl<xt::xarray<T>>::run(file, path, data);
        }

        return detail::xtensor::overwrite_impl<xt::xarray<T>>::run(file, path, data);
    }

    template <class T, std::size_t rank>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xtensor<T,rank>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !file.exist(path)))
        {
            return detail::xtensor::dump_impl<xt::xtensor<T,rank>>::run(file, path, data);
        }

        return detail::xtensor::overwrite_impl<xt::xtensor<T,rank>>::run(file, path, data);
    }
}

#endif // HIGHFIVE_XTENSOR

#endif // H5EASY_BITS_XTENSOR_HPP

