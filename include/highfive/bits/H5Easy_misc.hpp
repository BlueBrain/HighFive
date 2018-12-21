/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_BITS_MISC_HPP
#define H5EASY_BITS_MISC_HPP

#include "../H5Easy.hpp"

namespace HighFive
{
    namespace detail
    {
        inline auto error(const HighFive::File& file, const std::string& path, std::string message)
        {
            message += "\n";
            message += "path: '" + path + "'\n";
            message += "filename: '" + file.getName() + "'\n";

            return std::runtime_error(message);
        }
    }

    inline std::string groupName(const std::string& path)
    {
        std::size_t idx = path.find_last_of("/\\");

        if ( idx == std::string::npos )
        {
            return "/";
        }

        if ( idx == 0 )
        {
            return "/";
        }

        return path.substr(0,idx);
    }

    inline void createGroupsToDataSet(HighFive::File& file, const std::string& path)
    {
        std::string group_name = groupName(path);

        if ( not file.exist(group_name) )
        {
            file.createGroup(group_name);
        }
    }

    inline auto size(const HighFive::File& file, const std::string& path)
    {
        HighFive::DataSet dataset = file.getDataSet(path);

        auto dataspace = dataset.getSpace();

        auto dims = dataspace.getDimensions();

        using T = decltype(dims)::value_type;

        return std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<T>());
    }

    inline auto size(const HighFive::DataSet& dataset)
    {
        auto dataspace = dataset.getSpace();

        auto dims = dataspace.getDimensions();

        using T = decltype(dims)::value_type;

        return std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<T>());
    }

    inline auto shape(const HighFive::File& file, const std::string& path)
    {
        HighFive::DataSet dataset = file.getDataSet(path);

        auto dataspace = dataset.getSpace();

        return dataspace.getDimensions();
    }

    inline auto shape(const HighFive::DataSet& dataset)
    {
        auto dataspace = dataset.getSpace();

        return dataspace.getDimensions();
    }
}

#endif // H5EASY_BITS_MISC_HPP

