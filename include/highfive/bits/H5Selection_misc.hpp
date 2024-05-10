/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

namespace HighFive {

inline Selection::Selection(const DataSpace& memspace,
                            const DataSpace& file_space,
                            const DataSet& set)
    : _mem_space(memspace)
    , _file_space(file_space)
    , _set(set) {}

inline DataSpace Selection::getSpace() const {
    return _file_space;
}

inline DataSpace Selection::getMemSpace() const {
    return _mem_space;
}

inline DataSet& Selection::getDataset() {
    return _set;
}

inline const DataSet& Selection::getDataset() const {
    return _set;
}

// Not only a shortcut but also for templated compat with H5Dataset
inline const DataType Selection::getDataType() const {
    return _set.getDataType();
}

namespace detail {
inline Selection make_selection(const DataSpace& mem_space,
                                const DataSpace& file_space,
                                const DataSet& set) {
    return Selection(mem_space, file_space, set);
}
}  // namespace detail

}  // namespace HighFive
