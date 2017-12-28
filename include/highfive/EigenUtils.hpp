//
// Created by nwknoblauch on 12/20/17.
//

#ifndef HIGHFIVE_EIGENUTILS_HPP
#define HIGHFIVE_EIGENUTILS_HPP


#ifdef H5_USE_EIGEN

#ifdef USE_BLOSC

#include <H5Ppublic.h>
#include <highfive/H5Filter.hpp>
#include "blosc_filter.h"

#endif

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Utility.hpp>

namespace HighFive {


    bool is_transposed(const DataSet &dataset) {
        int doTranspose = 0;
        if (dataset.hasAttribute("doTranspose")) {
            dataset.getAttribute("doTranspose").read(doTranspose);
        }
        return (doTranspose != 0);
    }

    template<typename Scalar, int RowsAtCompileTime, int ColsAtCompileTime, int Options>
    void read_mat_h5(
            const std::string &file_name,
            const std::string &group_path,
            const std::string &data_name,
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> &retmat,
            std::vector<size_t> offsets = {0, 0},
            std::vector<size_t> chunk_size = {}) {


        typedef Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> mat;
        constexpr int transpose_type = Options == Eigen::RowMajor ? Eigen::ColMajor : Eigen::RowMajor;
        HighFive::File file(file_name, HighFive::File::ReadOnly);

        auto group = file.getGroup(group_path);


        auto dataset = group.getDataSet(data_name);
        bool transposed = is_transposed(dataset);
        auto disk_dims = dataset.getSpace().getDimensions();
        auto disk_chunk = disk_dims;
        auto data_chunk = chunk_size;
        std::vector<size_t> disk_offset = {0, 0};

        //If chunk_size is empty, use the dimensions of the dataset on disk
        // These will need to be transposed if the data is transposed
        // The disk chunk size should NOT be transposed, however
        if (chunk_size.empty()) {
            std::copy(disk_dims.begin(), disk_dims.end(), std::back_inserter(chunk_size));
            disk_chunk = chunk_size;
            if (transposed) {
                std::reverse(chunk_size.begin(), chunk_size.end());
            }
            offsets = {0, 0};
        } else {
            disk_offset = offsets;
            disk_chunk = chunk_size;
            //Do NOT transpose chunk size if chunk size is pre-specified
            // DO transpose disk_chunk size if chunk size is pre-specified
            if (transposed) {
                std::reverse(disk_offset.begin(), disk_offset.end());
                std::reverse(disk_chunk.begin(), disk_chunk.end());
            }
        }


        if (((size_t) retmat.rows() < chunk_size[0]) || ((size_t) retmat.cols() < chunk_size[1])) {
            //retmat must be resized so that the row and column number match the desired output row and column number
            retmat.resize(chunk_size[0], chunk_size[1]);
        }

        auto sel = dataset.select(offsets, disk_chunk, std::vector<size_t>());
        if (transposed) {
            Eigen::Map<Eigen::Matrix<Scalar, ColsAtCompileTime, RowsAtCompileTime, transpose_type> > temp_mat(
                    retmat.data(), disk_chunk[0], disk_chunk[1]);
            sel.read(temp_mat);
        } else {
            Eigen::Map<Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> > temp_mat(retmat.data(),
                                                                                                       disk_chunk[0],
                                                                                                       disk_chunk[1]);
            sel.read(temp_mat);
        }

    }

    template<typename Scalar, int RowsAtCompileTime, int ColsAtCompileTime, int Options>
    void write_mat_h5(
            const std::string &file_name,
            const std::string &group_path,
            const std::string &data_name,
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> &matrix) {


        typedef Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> mat;


        int transpose_int = 0;
        std::vector<Eigen::Index> map_dims(2, 0);
        map_dims[0] = matrix.rows();
        map_dims[1] = matrix.cols();
        assert(map_dims.size() == 2);

        HighFive::File file(file_name, HighFive::File::ReadWrite | HighFive::File::Create);


        Group group = file.createOrGetGroup(group_path);

        auto plist = H5Pcreate(H5P_DATASET_CREATE);

#ifdef USE_BLOSC
        typedef typename details::remove_const<mat>::type type_no_const;
        int r = 0;
        r = register_blosc(nullptr, nullptr);

        // Create a new file using the default property lists.
        const size_t dim_buffer = details::array_dims<type_no_const>::value;

        size_t chunkblocka = (size_t) (map_dims[0] < 1000 ? map_dims[0] : 1000);
        size_t chunkblockb = (size_t) (map_dims[1] < 1000 ? map_dims[1] : 1000);
        std::vector<size_t> cshape = {chunkblocka};
        if (dim_buffer == 2) {
            cshape.push_back(chunkblockb);
        }
        Filter filter(cshape, FILTER_BLOSC, r);
        // Create a dataset with double precision floating points
        plist = filter.getId();
#endif

        Eigen::Map<mat> w_mat(matrix.data(), map_dims[0], map_dims[1]);
        DataSpace ds = DataSpace::From(w_mat);

        DataSet dataset = group.createDataSet(data_name, ds, AtomicType<Scalar>(), plist);
        auto transpose_attr = dataset.createAttribute<int>("doTranspose", DataSpace::From(transpose_int));
        transpose_attr.write(transpose_int);
        dataset.write(w_mat);
    }


    template<typename Scalar, int RowsAtCompileTime, int ColsAtCompileTime, int Options>
    void write_mat_h5_transpose(
            const std::string &file_name,
            const std::string &group_path,
            const std::string &data_name,
            Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> &matrix) {


        typedef Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime, Options> mat;
        typedef std::conditional_t<std::is_same_v<std::integral_constant<int, Options>, std::integral_constant<int, Eigen::RowMajor> >,
                std::integral_constant<decltype(Eigen::ColMajor), Eigen::ColMajor>,
                std::integral_constant<decltype(Eigen::ColMajor), Eigen::RowMajor> > Transpose_Type;
        constexpr int mappedRows = ColsAtCompileTime;
        constexpr int mappedCols = RowsAtCompileTime;
        typedef Eigen::Matrix<Scalar, mappedRows, mappedCols, Transpose_Type::value> transpose_mat;


        int transpose_int = 1;
        std::vector<Eigen::Index> map_dims(2, 0);
        map_dims[0] = matrix.rows();
        map_dims[1] = matrix.cols();
        assert(map_dims.size() == 2);
        std::reverse(map_dims.begin(), map_dims.end());


        HighFive::File file(file_name, HighFive::File::ReadWrite | HighFive::File::Create);


        Group group = file.createOrGetGroup(group_path);


        auto plist = H5Pcreate(H5P_DATASET_CREATE);

#ifdef USE_BLOSC
        typedef typename details::remove_const<mat>::type type_no_const;
        int r = 0;
        r = register_blosc(nullptr, nullptr);

        // Create a new file using the default property lists.
        const size_t dim_buffer = details::array_dims<type_no_const>::value;

        size_t chunkblocka = (size_t) (map_dims[0] < 1000 ? map_dims[0] : 1000);
        size_t chunkblockb = (size_t) (map_dims[1] < 1000 ? map_dims[1] : 1000);
        std::vector<size_t> cshape = {chunkblocka};
        if (dim_buffer == 2) {
            cshape.push_back(chunkblockb);
        }
        Filter filter(cshape, FILTER_BLOSC, r);
        // Create a dataset with double precision floating points
        plist = filter.getId();
#endif

        Eigen::Map<transpose_mat> w_mat(matrix.data(), map_dims[0], map_dims[1]);
        DataSpace ds = DataSpace::From(w_mat);

        DataSet dataset = group.createDataSet(data_name, ds, AtomicType<Scalar>(), plist);
        auto transpose_attr = dataset.createAttribute<int>("doTranspose", DataSpace::From(transpose_int));
        transpose_attr.write(transpose_int);
        dataset.write(w_mat);
    }


#endif
}

#endif //HIGHFIVE_EIGENUTILS_HPP
