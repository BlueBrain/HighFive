#pragma once

#include "../bits/H5Inspector_decl.hpp"
#include "../H5Exception.hpp"

#include <opencv2/opencv.hpp>

#include "../bits/convert_size_vector.hpp"

namespace HighFive {
namespace details {


template <class T>
struct inspector<cv::Mat_<T>> {
    using type = cv::Mat_<T>;
    using value_type = T;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;

    static void assert_row_major(const type& type) {
        // Documentation claims that Mat_ is always row-major. However, it
        // could be padded. The steps/strides are in bytes.
        int rank = type.dims;
        size_t ld = sizeof(T);
        for (int i = rank - 1; i >= 0; --i) {
            if (static_cast<size_t>(type.step[i]) != ld) {
                throw DataSetException("Padded cv::Mat_ are not supported.");
            }

            ld *= static_cast<size_t>(type.size[i]);
        }
    }


    static constexpr size_t min_ndim = 2 + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = 1024 + inspector<value_type>::max_ndim;

    // HighFive doesn't support padded OpenCV arrays. Therefore, pretend
    // that they themselves are trivially copyable. And error out if the
    // assumption is violated.
    static constexpr bool is_trivially_copyable = std::is_trivially_copyable<value_type>::value &&
                                                  inspector<value_type>::is_trivially_nestable;
    static constexpr bool is_trivially_nestable = false;

    static size_t getRank(const type& val) {
        if (val.empty()) {
            return min_ndim;

        } else {
            return static_cast<size_t>(val.dims) +
                   inspector<value_type>::getRank(getAnyElement(val));
        }
    }

    static const T& getAnyElement(const type& val) {
        return *reinterpret_cast<T const*>(val.data);
    }

    static T& getAnyElement(type& val) {
        return *reinterpret_cast<T*>(val.data);
    }

    static size_t getLocalRank(const type& val) {
        return static_cast<size_t>(val.dims);
    }

    static std::vector<size_t> getDimensions(const type& val) {
        auto local_rank = getLocalRank(val);
        auto rank = getRank(val);
        std::vector<size_t> dims(rank, 1ul);

        if (val.empty()) {
            dims[0] = 0ul;
            dims[1] = 1ul;
            return dims;
        }

        for (size_t i = 0; i < local_rank; ++i) {
            dims[i] = static_cast<size_t>(val.size[static_cast<int>(i)]);
        }

        auto s = inspector<value_type>::getDimensions(getAnyElement(val));
        std::copy(s.cbegin(), s.cend(), dims.begin() + static_cast<int>(local_rank));
        return dims;
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        auto subdims = detail::convertSizeVector<int>(dims);
        val.create(static_cast<int>(subdims.size()), subdims.data());
    }

    static hdf5_type* data(type& val) {
        assert_row_major(val);

        if (!is_trivially_copyable) {
            throw DataSetException("Invalid used of `inspector<Eigen::Matrix<...>>::data`.");
        }

        if (val.empty()) {
            return nullptr;
        }

        return inspector<value_type>::data(getAnyElement(val));
    }

    static const hdf5_type* data(const type& val) {
        assert_row_major(val);

        if (!is_trivially_copyable) {
            throw DataSetException("Invalid used of `inspector<Eigen::Matrix<...>>::data`.");
        }

        if (val.empty()) {
            return nullptr;
        }

        return inspector<value_type>::data(getAnyElement(val));
    }

    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* m) {
        if (val.empty()) {
            return;
        }

        auto local_rank = val.dims;
        auto subdims = std::vector<size_t>(dims.begin() + local_rank, dims.end());
        auto subsize = compute_total_size(subdims);
        for (auto it = val.begin(); it != val.end(); ++it) {
            inspector<value_type>::serialize(*it, subdims, m);
            m += subsize;
        }
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        auto local_rank = val.dims;
        auto subdims = std::vector<size_t>(dims.begin() + local_rank, dims.end());
        auto subsize = compute_total_size(subdims);
        for (auto it = val.begin(); it != val.end(); ++it) {
            inspector<value_type>::unserialize(vec_align, subdims, *it);
            vec_align += subsize;
        }
    }
};

}  // namespace details
}  // namespace HighFive
