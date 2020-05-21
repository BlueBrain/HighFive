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

///
/// Get the parent of a path.
/// For example for ``path = "/path/to/dataset"`` this function returns
/// ``"/path/to"``.
///
/// \param path path to a DataSet
///
/// \return group the path of the group above the DataSet
inline std::string getParentName(const std::string& path) {
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
inline void createGroupsToDataSet(File& file, const std::string& path) {
    std::string group_name = getParentName(path);
    if (!file.exist(group_name)) {
        file.createGroup(group_name);
    }
}

// Generate error-stream and return "Exception" (not yet thrown).
inline Exception error(const File& file,
                       const std::string& path,
                       const std::string& message) {
    std::ostringstream ss;
    ss << message << std::endl
       << "Path: " << path << std::endl
       << "Filename: " << file.getName() << std::endl;
    return Exception(ss.str());
}

// Generate specific dump error
inline Exception dump_error(File& file, const std::string& path)
{
    if (file.getObjectType(path) == ObjectType::Dataset) {
        return error(file, path,
            "H5Easy: Dataset already exists, dump with H5Easy::DumpMode::Overwrite "
            "to overwrite (with an array of the same shape).");
    } else {
        return error(file, path,
            "H5Easy: path exists, but does not correspond to a Dataset. Dump not possible.");
    }
}

// structure to pass-on dump-settings
struct DumpSettings
{
    inline DumpSettings() = default;

    inline void set(DumpMode mode)
    {
        if (mode == DumpMode::Overwrite) {
            overwrite = true;
        } else if (mode == DumpMode::Create) {
            overwrite = false;
        }
    }

    inline void set(Compression mode)
    {
        if (mode == Compression::None) {
            compress = false;
        } else if (mode == Compression::Medium) {
            compress = true;
            deflate_level = 5;
        } else if (mode == Compression::High) {
            compress = true;
            deflate_level = 9;
        }
    }

    bool overwrite = false;
    bool compress = false;
    unsigned deflate_level = 0;
};

// variadic template to read the DumpSettings
inline void read_dumpsettings(DumpSettings&)
{
}

template <class T, class... Args>
inline void read_dumpsettings(DumpSettings& options, T arg, Args... args)
{
    options.set(arg);
    read_dumpsettings(options, args...);
}

// process DumpSettings
template <class... Args>
inline DumpSettings get_dumpsettings(Args... args)
{
    DumpSettings out;
    read_dumpsettings(out, args...);
    return out;
}

// get a opened DataSet: nd-array
template <class T>
inline DataSet init_dataset(File& file,
                            const std::string& path,
                            const std::vector<size_t>& shape,
                            const DumpSettings& settings)
{
    if (!file.exist(path)) {
        detail::createGroupsToDataSet(file, path);
        if (settings.compress == false) {
            return file.createDataSet<T>(path, DataSpace(shape));
        } else {
            std::vector<hsize_t> hshape(shape.begin(), shape.end());
            DataSetCreateProps props;
            props.add(Chunking(hshape));
            props.add(Shuffle());
            props.add(Deflate(settings.deflate_level));
            return file.createDataSet<T>(path, DataSpace(shape), props);
        }
    } else if (settings.overwrite && file.getObjectType(path) == ObjectType::Dataset) {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getDimensions() != shape) {
            throw detail::error(file, path, "H5Easy::dump: Inconsistent dimensions");
        }
        return dataset;
    }
    throw dump_error(file, path);
}

// get a opened DataSet: scalar
template <class T>
inline DataSet init_dataset(File& file,
                            const std::string& path,
                            const T& data,
                            const DumpSettings& settings)
{
    if (!file.exist(path)) {
        detail::createGroupsToDataSet(file, path);
        return file.createDataSet<T>(path, DataSpace::From(data));
    } else if (settings.overwrite && file.getObjectType(path) == ObjectType::Dataset) {
        DataSet dataset = file.getDataSet(path);
        if (dataset.getElementCount() != 1) {
            throw detail::error(file, path, "H5Easy::dump: Existing field not a scalar");
        }
        return dataset;
    }
    throw dump_error(file, path);
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
