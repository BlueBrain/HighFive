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

namespace H5Easy {

namespace detail {

// Generate error-stream and return "Exception" (not yet thrown).
inline Exception error(const File& file,
                       const std::string& path,
                       const std::string& message)
{
    std::ostringstream ss;
    ss << message << std::endl
       << "Path: " << path << std::endl
       << "Filename: " << file.getName() << std::endl;
    return Exception(ss.str());
}

///
/// Get the parent of a path.
/// For example for ``path = "/path/to/dataset"`` this function returns
/// ``"/path/to"``.
///
/// \param path path to a DataSet
///
/// \return group the path of the group above the DataSet
inline std::string getParentName(const std::string& path)
{
    std::size_t idx = path.find_last_of("/\\");

    if (idx == std::string::npos) {
        return "/";
    } else if (idx == 0) {
        return "/";
    } else {
        return path.substr(0, idx);
    }
}

///
/// \brief Recursively create groups in an open HDF5 file such that a
/// \a DataSet can be created (see ``getParentName``).
///
/// \param file opened File
/// \param path path of the DataSet
///
inline void createGroupsToDataSet(File& file, const std::string& path)
{
    std::string group_name = getParentName(path);

    if (not file.exist(group_name)) {
        file.createGroup(group_name);
    }
}

}  // namespace detail

inline size_t getSize(const File& file, const std::string& path) {
    return file.getDataSet(path).getElementCount();
}

inline std::vector<size_t> getShape(const File& file, const std::string& path) {
    return file.getDataSet(path).getDimensions();
}

}  // namespace H5Easy

#endif  // H5EASY_BITS_MISC_HPP
