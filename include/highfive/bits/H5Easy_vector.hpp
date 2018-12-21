/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_VECTOR_HPP
#define H5EASY_BITS_VECTOR_HPP

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"

namespace HighFive
{
    namespace detail
    {
        namespace vector
        {
            template <class T, class E = void>
            struct dump_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const std::vector<C>& data)
                {
                    createGroupsToDataSet(file, path);

                    HighFive::DataSet dataset = file.createDataSet<T>(path, HighFive::DataSpace::From(data));

                    dataset.write(data);

                    file.flush();

                    return dataset;
                }
            };

            template <class T, class E = void>
            struct overwrite_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                {
                    HighFive::DataSet dataset = file.getDataSet(path);

                    auto dims = shape(dataset);

                    if (dims.size() > 1)
                    {
                        throw detail::error(file, path, "HighFive::dump: Can only overwrite 1-d vectors");
                    }

                    if (dims[0] != data.size())
                    {
                        throw detail::error(file, path, "HighFive::dump: Inconsistent dimensions");
                    }

                    dataset.write(data);

                    file.flush();

                    return dataset;
                }
            };
        }

        template <class T>
        struct load_impl<std::vector<T>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                HighFive::DataSet dataset = file.getDataSet(path);

                auto dims = shape(dataset);

                if (dims.size() != 1)
                {
                    throw detail::error(file, path, "HighFive::load: Field not rank 1");
                }

                std::vector<T> data;

                dataset.read(data);

                return data;
            }
        };
    }

    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const std::vector<T>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !file.exist(path)))
        {
            return detail::vector::dump_impl<T>::run(file, path, data);
        }

        return detail::vector::overwrite_impl<T>::run(file, path, data);
    }

}

#endif // H5EASY_BITS_VECTOR_HPP

