/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5EASY_HPP
#define H5EASY_HPP

#include <string>
#include <vector>

// optionally enable xtensor plug-in and load the library
#ifdef XTENSOR_VERSION_MAJOR
#ifndef H5_USE_XTENSOR
#define H5_USE_XTENSOR
#endif
#endif

#ifdef H5_USE_XTENSOR
#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>
#endif

// optionally enable Eigen plug-in and load the library
#ifdef EIGEN_WORLD_VERSION
#ifndef H5_USE_EIGEN
#define H5_USE_EIGEN
#endif
#endif

#ifdef H5_USE_EIGEN
#include <Eigen/Eigen>
#endif

// optionally enable OpenCV plug-in and load the library
#ifdef CV_MAJOR_VERSION
#ifndef H5_USE_OPENCV
#define H5_USE_OPENCV
#endif
#endif

#ifdef H5_USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

#include "H5File.hpp"

namespace H5Easy {

using HighFive::Attribute;
using HighFive::AtomicType;
using HighFive::Chunking;
using HighFive::DataSet;
using HighFive::DataSetCreateProps;
using HighFive::DataSpace;
using HighFive::Deflate;
using HighFive::Exception;
using HighFive::File;
using HighFive::ObjectType;
using HighFive::Shuffle;

///
/// \brief Write mode for DataSets
enum class DumpMode {
    Create = 0, /*!< Dump only if DataSet does not exist, otherwise throw. */
    Overwrite = 1 /*!< Create or overwrite if DataSet of correct shape exists, otherwise throw. */
};

///
/// \brief Enable/disable automatic flushing after write operations.
enum class Flush
{
    False = 0, /*!< No automatic flushing. */
    True = 1 /*!< Automatic flushing. */
};

///
/// \brief Set compression level for written DataSets.
class Compression
{
public:

    //
    // \brief Enable compression with the highest compression level (9).
    // or disable compression (set compression level to 0).
    explicit Compression(bool enable = true);

    //
    // \brief Set compression level.
    template <class T>
    Compression(T level);

    //
    // \brief Return compression level.
    inline unsigned get() const;

private:
    unsigned m_compression_level;
};

///
/// \brief Options for dumping data.
///
/// By default:
/// - DumpMode::Create
/// - Flush::True
/// - Compression(false)
/// - ChunkSize: automatic.
class DumpOptions
{
public:
    ///
    /// \brief Constructor: accept all defaults.
    DumpOptions() = default;

    ///
    /// \brief Constructor: overwrite (some of the) defaults.
    /// \param Any of DumpMode, Flush, Compression in arbitrary number and order.
    template <class... Args>
    DumpOptions(Args... args)
    {
        set(args...);
    }

    ///
    /// \brief Overwrite setting.
    /// \param mode: DumpMode.
    inline void set(DumpMode mode);

    ///
    /// \brief Overwrite setting.
    /// \param flush Flush.
    inline void set(Flush mode);

    ///
    /// \brief Overwrite setting.
    /// \param level Compression.
    inline void set(const Compression& level);

    ///
    /// \brief Overwrite settings.
    /// \param Any of DumpMode, Flush, Compression in arbitrary number and order.
    template <class T, class... Args>
    inline void set(T arg, Args... args);

    ///
    /// \brief Set chunk-size. If the input is rank (size) zero, automatic chunking is enabled.
    /// \param shape Chunk size along each dimension.
    template <class T>
    inline void setChunkSize(const std::vector<T>& shape);

    ///
    /// \brief Set chunk-size. If the input is rank (size) zero, automatic chunking is enabled.
    /// \param shape Chunk size along each dimension.
    inline void setChunkSize(std::initializer_list<size_t> shape);

    ///
    /// \brief Check to overwrite.
    inline bool overwrite() const;

    ///
    /// \brief Check to flush.
    inline bool flush() const;

    ///
    /// \brief Check to compress.
    inline bool compress() const;

    ///
    /// \brief Get compression level.
    inline unsigned getCompressionLevel() const;

    ///
    /// \brief Check if chunk-size is manually set (or should be computed automatically).
    inline bool isChunked() const;

    ///
    /// \brief Get chunk size.
    inline std::vector<hsize_t> getChunkSize() const;

private:
    bool m_overwrite = false;
    bool m_flush = true;
    unsigned m_compression_level = 0;
    std::vector<hsize_t> m_chunk_size = {};
};

///
/// \brief Get the size of an existing DataSet in an open HDF5 file.
///
/// \param file A readable opened file
/// \param path Path of the DataSet
///
/// \return Size of the DataSet
inline size_t getSize(const File& file, const std::string& path);

///
/// \brief Get the shape of an existing DataSet in an readable file.
///
/// \param file A readable opened file
/// \param path Path of the DataSet
///
/// \return the shape of the DataSet
inline std::vector<size_t> getShape(const File& file, const std::string& path);

///
/// \brief Write object (templated) to a (new) DataSet in an open HDF5 file.
///
/// \param file Writeable opened file
/// \param path Path of the DataSet
/// \param data Data to write
/// \param mode Write mode
///
/// \return The newly created DataSet
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    DumpMode mode = DumpMode::Create);

///
/// \brief Write object (templated) to a (new) DataSet in an open HDF5 file.
///
/// \param file Writeable opened file
/// \param path Path of the DataSet
/// \param data Data to write
/// \param options Dump options
///
/// \return The newly created DataSet
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const DumpOptions& options);

///
/// \brief Write a scalar to a (new, extendible) DataSet in an open HDF5 file.
///
/// \param file opened File (has to be writeable)
/// \param path path of the DataSet
/// \param data the data to write
/// \param idx the indices to which to write
///
/// \return The newly created DataSet
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::vector<size_t>& idx);

///
/// \brief Write a scalar to a (new, extendable) DataSet in an open HDF5 file.
///
/// \param file open File (has to be writeable)
/// \param path path of the DataSet
/// \param data the data to write
/// \param idx the indices to which to write
///
/// \return The newly created DataSet
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::initializer_list<size_t>& idx);

///
/// \brief Write a scalar to a (new, extendible) DataSet in an open HDF5 file.
///
/// \param file opened File (has to be writeable)
/// \param path path of the DataSet
/// \param data the data to write
/// \param idx the indices to which to write
/// \param options Dump options
///
/// \return The newly created DataSet
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::vector<size_t>& idx,
                    const DumpOptions& options);

///
/// \brief Write a scalar to a (new, extendible) DataSet in an open HDF5 file.
///
/// \param file opened File (has to be writeable)
/// \param path path of the DataSet
/// \param data the data to write
/// \param idx the indices to which to write
/// \param options Dump options
///
/// \return The newly created DataSet
///
template <class T>
inline DataSet dump(File& file,
                    const std::string& path,
                    const T& data,
                    const std::initializer_list<size_t>& idx,
                    const DumpOptions& options);

///
/// \brief Load entry "(i,j)" from a rank-two DataSet in an open HDF5 file to a scalar.
///
/// \param file opened File (has to be writeable)
/// \param idx the indices to load
/// \param path path of the DataSet
///
/// \return the read data
///
template <class T>
inline T load(const File& file, const std::string& path, const std::vector<size_t>& idx);

///
/// \brief Load a DataSet in an open HDF5 file to an object (templated).
///
/// \param file opened File (has to be writeable)
/// \param path path of the DataSet
///
/// \return the read data
///
template <class T>
inline T load(const File& file, const std::string& path);

///
/// \brief Write object (templated) to a (new) Attribute in an open HDF5 file.
///
/// \param file Writeable opened file
/// \param path Path of the DataSet
/// \param key Name of the attribute
/// \param data Data to write
/// \param mode Write mode
///
/// \return The newly created DataSet
///
template <class T>
inline Attribute dumpAttribute(File& file,
                               const std::string& path,
                               const std::string& key,
                               const T& data,
                               DumpMode mode = DumpMode::Create);

///
/// \brief Write object (templated) to a (new) Attribute in an open HDF5 file.
///
/// \param file Writeable opened file
/// \param path Path of the DataSet
/// \param key Name of the attribute
/// \param data Data to write
/// \param options Dump options
///
/// \return The newly created DataSet
///
template <class T>
inline Attribute dumpAttribute(File& file,
                               const std::string& path,
                               const std::string& key,
                               const T& data,
                               const DumpOptions& options);

///
/// \brief Load a Attribute in an open HDF5 file to an object (templated).
///
/// \param file opened File (has to be writeable)
/// \param path path of the DataSet
/// \param key Name of the attribute
///
/// \return the read data
///
template <class T>
inline T loadAttribute(const File& file, const std::string& path, const std::string& key);

}  // namespace H5Easy

#include "h5easy_bits/H5Easy_Eigen.hpp"
#include "h5easy_bits/H5Easy_misc.hpp"
#include "h5easy_bits/H5Easy_opencv.hpp"
#include "h5easy_bits/H5Easy_public.hpp"
#include "h5easy_bits/H5Easy_scalar.hpp"
#include "h5easy_bits/H5Easy_vector.hpp"
#include "h5easy_bits/H5Easy_xtensor.hpp"

#endif  // H5EASY_HPP
