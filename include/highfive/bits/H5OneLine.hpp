/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#ifndef H5ONELINE_BITS_HPP
#define H5ONELINE_BITS_HPP

#include "../H5OneLine.hpp"

namespace HighFive
{
    inline bool exist(const HighFive::File& file, const std::string& path)
    {
        // find first "/"
        std::size_t idx = path.find("/");

        // loop over all groups
        while (true)
        {
            // terminate if all "/" have been found
            if (std::string::npos == idx)
            {
                break;
            }

            // create group if needed
            if (idx > 0)
            {
                // get group name
                std::string name(path.substr(0,idx));
                // create if needed
                if (!file.exist(name))
                {
                    return false;
                }
            }

            // proceed to next "/"
            idx = path.find("/",idx+1);
        }

        return file.exist(path);
    }

    inline void createGroup(HighFive::File& file, const std::string& path)
    {
        // find first "/"
        std::size_t idx = path.find("/");

        // loop over all groups
        while (true)
        {
            // terminate if all "/" have been found
            if (std::string::npos == idx)
            {
                return;
            }

            // create group if needed
            if (idx > 0)
            {
                // get group name
                std::string name(path.substr(0,idx));

                // create if needed
                if (!file.exist(name))
                {
                    file.createGroup(name);
                }
            }

            // proceed to next "/"
            idx = path.find("/",idx+1);
        }
    }

    inline std::size_t size(const HighFive::File& file, const std::string& path)
    {
        if (!exist(file, path))
        {
            throw std::runtime_error("HighFive::size: Field does not exist ('"+path+"')");
        }

        HighFive::DataSet dataset = file.getDataSet(path);

        auto dataspace = dataset.getSpace();

        auto dims = dataspace.getDimensions();

        std::size_t size = 1;

        for (std::size_t i = 0; i < dims.size(); ++i)
        {
            size *= dims[i];
        }

        return size;
    }

    inline std::vector<std::size_t> shape(const HighFive::File& file, const std::string& path)
    {
        if (!exist(file, path))
        {
            throw std::runtime_error("HighFive::shape: Field does not exist ('"+path+"')");
        }

        HighFive::DataSet dataset = file.getDataSet(path);

        auto dataspace = dataset.getSpace();

        auto dims = dataspace.getDimensions();

        std::vector<std::size_t> shape(dims.size());

        for (std::size_t i = 0; i < dims.size(); ++i)
        {
            shape[i] = dims[i];
        }

        return shape;
    }

    namespace detail
    {
        namespace scalar
        {
            template <class T, class E = void>
            struct dump_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                {
                    createGroup(file, path);

                    HighFive::DataSet dataset = file.createDataSet<C>(path, HighFive::DataSpace::From(data));

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

                    auto dataspace = dataset.getSpace();

                    auto dims = dataspace.getDimensions();

                    if (dims.size() != 0)
                    {
                        throw std::runtime_error("HighFive::dump: Existing field not a scalar ('"+path+"')");
                    }

                    dataset.write(data);

                    file.flush();

                    return dataset;
                }
            };

            template <class T, class E = void>
            struct dump_extend_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path,
                    const C& data, const std::vector<std::size_t>& idx)
                {
                    std::vector<std::size_t> ones(idx.size(), 1);

                    if (exist(file, path))
                    {
                        HighFive::DataSet dataset = file.getDataSet(path);

                        auto dataspace = dataset.getSpace();

                        auto dims = dataspace.getDimensions();

                        auto shape = dims;

                        if (dims.size() != idx.size())
                        {
                            throw std::runtime_error("xt::dump: Rank of the index and the existing field do not match ('"+path+"')");
                        }

                        for (std::size_t i = 0; i < dims.size(); ++i)
                        {
                            shape[i] = std::max(dims[i], idx[i]+1);
                        }

                        if (shape != dims)
                        {
                            dataset.resize(shape);
                        }

                        dataset.select(idx, ones).write(data);

                        file.flush();

                        return dataset;
                    }

                    createGroup(file, path);

                    auto shape = idx;

                    const size_t unlim = HighFive::DataSpace::UNLIMITED;
                    std::vector<std::size_t> unlim_shape(idx.size(), unlim);

                    std::vector<hsize_t> chuncks(idx.size(), 10);

                    for (auto& i: shape)
                    {
                        i++;
                    }

                    HighFive::DataSpace dataspace = HighFive::DataSpace(shape, unlim_shape);

                    HighFive::DataSetCreateProps props;
                    props.add(HighFive::Chunking(chuncks));

                    HighFive::DataSet dataset = file.createDataSet(path, dataspace, HighFive::AtomicType<T>(), props);

                    dataset.select(idx, ones).write(data);

                    file.flush();

                    return dataset;
                }
            };

            template <class T>
            struct load_impl
            {
                static auto run(const HighFive::File& file, const std::string& path, const std::vector<std::size_t>& idx)
                {
                    std::vector<std::size_t> ones(idx.size(), 1);

                    HighFive::DataSet dataset = file.getDataSet(path);

                    T data;

                    dataset.select(idx, ones).read(data);

                    return data;
                }
            };
        }

        namespace vector
        {
            template <class T, class E = void>
            struct dump_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const std::vector<C>& data)
                {
                    createGroup(file, path);

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

                    auto dataspace = dataset.getSpace();

                    auto dims = dataspace.getDimensions();

                    if (dims.size() > 1)
                    {
                        throw std::runtime_error("HighFive::dump: Can only overwrite 1-d vectors ('"+path+"')");
                    }

                    if (dims[0] != data.size())
                    {
                        throw std::runtime_error("HighFive::dump: Inconsistent dimensions ('"+path+"')");
                    }

                    dataset.write(data);

                    file.flush();

                    return dataset;
                }
            };
        }

        #ifdef HIGHFIVE_EIGEN
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
                    createGroup(file, path);

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
                        throw std::runtime_error("HighFive::dump: Inconsistent rank ('"+path+"')");
                    }

                    for (std::size_t i = 0; i < shape.size(); ++i)
                    {
                        if (shape[i] != dims[i])
                        {
                            throw std::runtime_error("HighFive::dump: Inconsistent dimensions ('"+path+"')");
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
                        throw std::runtime_error("HighFive::load: Inconsistent rank ('"+path+"')");
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
        #endif

        #ifdef HIGHFIVE_XTENSOR
        namespace xtensor
        {
            template<class T>
            struct dump_impl
            {
                template <class C>
                static HighFive::DataSet run(HighFive::File& file, const std::string& path, const C& data)
                {
                    createGroup(file, path);

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
                        throw std::runtime_error("HighFive::dump: Inconsistent rank ('"+path+"')");
                    }

                    for (std::size_t i = 0; i < data.shape().size(); ++i)
                    {
                        if (data.shape()[i] != dims[i])
                        {
                            throw std::runtime_error("HighFive::dump: Inconsistent dimensions ('"+path+"')");
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
        #endif

        // scalar/string
        template <class T, class E = void>
        struct load_impl
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                HighFive::DataSet dataset = file.getDataSet(path);

                auto dataspace = dataset.getSpace();

                auto dims = dataspace.getDimensions();

                if (dims.size() != 0)
                {
                    throw std::runtime_error("HighFive::load: Field not a scalar ('"+path+"')");
                }

                T data;

                dataset.read(data);

                return data;
            }
        };

        template <class T>
        struct load_impl<std::vector<T>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                HighFive::DataSet dataset = file.getDataSet(path);

                auto dataspace = dataset.getSpace();

                auto dims = dataspace.getDimensions();

                if (dims.size() != 1)
                {
                    throw std::runtime_error("HighFive::load: Field not rank 1 ('"+path+"')");
                }

                std::vector<T> data;

                dataset.read(data);

                return data;
            }
        };

        #ifdef HIGHFIVE_EIGEN
        template <class T, int Rows, int Cols, int Options>
        struct load_impl<Eigen::Matrix<T,Rows,Cols,Options>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::eigen::load_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path);
            }
        };
        #endif

        #ifdef HIGHFIVE_XTENSOR
        template <class T>
        struct load_impl<xt::xarray<T>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::xtensor::load_impl<xt::xarray<T>>::run(file, path);
            }
        };
        #endif

        #ifdef HIGHFIVE_XTENSOR
        template <class T, std::size_t rank>
        struct load_impl<xt::xtensor<T,rank>>
        {
            static auto run(const HighFive::File& file, const std::string& path)
            {
                return detail::xtensor::load_impl<xt::xtensor<T,rank>>::run(file, path);
            }
        };
        #endif
    }

    #ifdef HIGHFIVE_EIGEN
    template <class T, int Rows, int Cols, int Options>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const Eigen::Matrix<T,Rows,Cols,Options>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !exist(file, path)))
        {
            return detail::eigen::dump_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path, data);
        }

        return detail::eigen::overwrite_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path, data);
    }
    #endif

    #ifdef HIGHFIVE_XTENSOR
    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xarray<T>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !exist(file, path)))
        {
            return detail::xtensor::dump_impl<xt::xarray<T>>::run(file, path, data);
        }

        return detail::xtensor::overwrite_impl<xt::xarray<T>>::run(file, path, data);
    }
    #endif

    #ifdef HIGHFIVE_XTENSOR
    template <class T, std::size_t rank>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const xt::xtensor<T,rank>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !exist(file, path)))
        {
            return detail::xtensor::dump_impl<xt::xtensor<T,rank>>::run(file, path, data);
        }

        return detail::xtensor::overwrite_impl<xt::xtensor<T,rank>>::run(file, path, data);
    }
    #endif

    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const std::vector<T>& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !exist(file, path)))
        {
            return detail::vector::dump_impl<T>::run(file, path, data);
        }

        return detail::vector::overwrite_impl<T>::run(file, path, data);
    }

    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data, HighFive::Mode mode)
    {
        if ((mode == HighFive::Mode::Create) or (mode == HighFive::Mode::Overwrite and !exist(file, path)))
        {
            return detail::scalar::dump_impl<T>::run(file, path, data);
        }

        return detail::scalar::overwrite_impl<T>::run(file, path, data);
    }

    template <class T>
    inline HighFive::DataSet dump(HighFive::File& file, const std::string& path, const T& data, const std::vector<std::size_t>& idx)
    {
        return detail::scalar::dump_extend_impl<T>::run(file, path, data, idx);
    }

    #ifdef HIGHFIVE_EIGEN
    template <class T, int Rows, int Cols, int Options>
    inline HighFive::DataSet overwrite(HighFive::File& file, const std::string& path, const Eigen::Matrix<T,Rows,Cols,Options>& data)
    {
        return detail::eigen::overwrite_impl<Eigen::Matrix<T,Rows,Cols,Options>>::run(file, path, data);
    }
    #endif

    template <class T>
    inline auto load(const HighFive::File& file, const std::string& path, const std::vector<std::size_t>& idx)
    {
        return detail::scalar::load_impl<T>::run(file, path, idx);
    }

    template <class T>
    inline auto load(const HighFive::File& file, const std::string& path)
    {
        return detail::load_impl<T>::run(file, path);
    }
}

#endif  // H5ONELINE_HPP

