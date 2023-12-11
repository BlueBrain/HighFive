/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <cstdlib>
#include <sstream>

#include "h5_wrapper.hpp"
#include "h5e_wrapper.hpp"

namespace HighFive {

struct HDF5ErrMapper {
    template <typename ExceptionType>
    static inline herr_t stackWalk(unsigned n, const H5E_error2_t* err_desc, void* client_data) {
        auto** e_iter = static_cast<ExceptionType**>(client_data);
        (void) n;

        const char* major_err = detail::nothrow::h5e_get_major(err_desc->maj_num);
        const char* minor_err = detail::nothrow::h5e_get_minor(err_desc->min_num);

        std::ostringstream oss;
        oss << '(' << major_err << ") " << minor_err;

        detail::nothrow::h5_free_memory((void*) major_err);
        detail::nothrow::h5_free_memory((void*) minor_err);

        auto* e = new ExceptionType(oss.str());
        e->_err_major = err_desc->maj_num;
        e->_err_minor = err_desc->min_num;
        (*e_iter)->_next.reset(e);
        *e_iter = e;
        return 0;
    }

    template <typename ExceptionType>
    [[noreturn]] static inline void ToException(const std::string& prefix_msg) {
        hid_t err_stack = H5Eget_current_stack();
        if (err_stack >= 0) {
            ExceptionType e("");
            ExceptionType* e_iter = &e;

            detail::nothrow::h5e_walk2(err_stack,
                                       H5E_WALK_UPWARD,
                                       &HDF5ErrMapper::stackWalk<ExceptionType>,
                                       &e_iter);
            detail::nothrow::h5e_clear2(err_stack);

            const char* next_err_msg = (e.nextException() != NULL) ? (e.nextException()->what())
                                                                   : ("");

            e.setErrorMsg(prefix_msg + " " + next_err_msg);
            throw e;
        }
        // throw generic error, unrecognized error
        throw ExceptionType(prefix_msg + ": Unknown HDF5 error");
    }
};

}  // namespace HighFive
