/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_EIGEN_HPP
#define H5EASY_BITS_EIGEN_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"

#ifdef HIGHFIVE_EIGEN

namespace HighFive
{
    namespace detail
    {
        namespace eigen
        {
            template <class C, int Rows, int Cols, int Options>
            std::vector<std::size_t> shape(const Eigen::Matrix<C,Rows,Cols,Options>& data)
            {
                if (Rows==1)
                {
                    return {static_cast<std::size_t>(data.cols())};
                }

                if (Cols==1)
                {
                    return {static_cast<std::size_t>(data.rows())};
                }

                return {static_cast<std::size_t>(data.rows()), static_cast<std::size_t>(data.cols())};
            }

            template <class C, int Rows, int Cols, int Options>
            void write(HighFive::DataSet& dataset, const Eigen::Matrix<C,Rows,Cols,Options>& data)
            {
                if (data.IsRowMajor)
                {
                    dataset.write(data.data());
                }
                else
                {
                    Eigen::Matrix<C,Rows,Cols,Eigen::RowMajor> tmp = data;

                    detail::eigen::write(dataset, tmp);
                }
            }

            template<class T>
            struct dump_impl
            {
                template <class C, int Rows, int Cols, int Options>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const Eigen::Matrix<C,Rows,Cols,Options>& data)
                {
                    createGroupsToDataSet(file, path);

                    std::vector<std::size_t> dims = detail::eigen::shape(data);

                    HighFive::DataSet dataset = file.createDataSet<C>(path, HighFive::DataSpace(dims));

                    detail::eigen::write(dataset, data);

                    file.flush();

                    return dataset;
                }
            };

            template<class T>
            struct overwrite_impl
            {
                template <class C, int Rows, int Cols, int Options>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const Eigen::Matrix<C,Rows,Cols,Options>& data)
                {
                    HighFive::DataSet dataset = file.getDataSet(path);

                    auto dataspace = dataset.getSpace();

                    auto dims = dataspace.getDimensions();

                    auto shape = detail::eigen::shape(data);

                    if (shape.size() != dims.size())
                    {
                        throw detail::error(file, path, "HighFive::dump: Inconsistent rank");
                    }

                    for (std::size_t i = 0; i < shape.size(); ++i)
                    {
                        if (shape[i] != dims[i])
                        {
                            throw detail::error(file, path, "HighFive::dump: Inconsistent dimensions");
                        }
                    }

                    detail::eigen::write(dataset, data);

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

                    T data;

                    if (dims.size()==1)
                    {
                        data.resize(dims[0], 1);
                    }
                    else if (dims.size()==2)
                    {
                        data.resize(dims[0], dims[1]);
                    }
                    else
                    {
                        throw detail::error(file, path, "HighFive::load: Inconsistent rank");
                    }

                    if (data.IsRowMajor)
                    {
                        dataset.read(data.data());
                    }
                    else
                    {
                        std::vector<typename T::Scalar> tmp(data.rows()*data.cols());

                        dataset.read(tmp.data());

                        for (auto i = 0; i < data.rows(); ++i)
                        {
                            for (auto j = 0; j < data.cols(); ++j)
                            {
                                data(i,j) = tmp[i*data.cols()+j];
                            }
                        }
                    }

                    return data;
                }
            };
        }

        template <class T, int Rows, int Cols, int Options>
        struct load_impl<Eigen::Matrix<T,Rows,Cols,Options>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::eigen::load_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path);
            }
        };
    }

    template <class T, int Rows, int Cols, int Options>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const Eigen::Matrix<T,Rows,Cols,Options>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !file.exist(path)))
        {
            return detail::eigen::dump_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path, data);
        }

        return detail::eigen::overwrite_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path, data);
    }

    template <class T, int Rows, int Cols, int Options>
    inline HighFive::DataSet overwrite(HighFive::File& file, const std::string& path, const Eigen::Matrix<T,Rows,Cols,Options>& data)
    {
        return detail::eigen::overwrite_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path, data);
    }
}

#endif // HIGHFIVE_EIGEN

#endif // H5EASY_BITS_EIGEN_HPP

