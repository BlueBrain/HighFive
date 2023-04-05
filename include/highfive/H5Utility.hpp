/*
 *  Copyright (c), 2017, Blue Brain Project - EPFL (CH)
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#pragma once

#include <H5Epublic.h>
#include <functional>
#include <string>
#include <iostream>

#include "bits/H5Friends.hpp"

namespace HighFive {

///
/// \brief Utility class to disable HDF5 stack printing inside a scope.
///
class SilenceHDF5 {
  public:
    inline SilenceHDF5(bool enable = true)
        : _client_data(nullptr) {
        H5Eget_auto2(H5E_DEFAULT, &_func, &_client_data);
        if (enable)
            H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
    }

    inline ~SilenceHDF5() {
        H5Eset_auto2(H5E_DEFAULT, _func, _client_data);
    }

  private:
    H5E_auto2_t _func;
    void* _client_data;
};

#define HIGHFIVE_LOG_LEVEL_DEBUG 10
#define HIGHFIVE_LOG_LEVEL_INFO  20
#define HIGHFIVE_LOG_LEVEL_WARN  30
#define HIGHFIVE_LOG_LEVEL_ERROR 40

#ifndef HIGHFIVE_LOG_LEVEL
#define HIGHFIVE_LOG_LEVEL HIGHFIVE_LOG_LEVEL_WARN
#endif

enum class LogSeverity {
    Debug = HIGHFIVE_LOG_LEVEL_DEBUG,
    Info = HIGHFIVE_LOG_LEVEL_INFO,
    Warn = HIGHFIVE_LOG_LEVEL_WARN,
    Error = HIGHFIVE_LOG_LEVEL_ERROR
};

inline std::string to_string(LogSeverity severity) {
    switch (severity) {
    case LogSeverity::Debug:
        return "DEBUG";
    case LogSeverity::Info:
        return "INFO";
    case LogSeverity::Warn:
        return "WARN";
    case LogSeverity::Error:
        return "ERROR";
    default:
        return "??";
    }
}

/** \brief A logger with supporting basic functionality.
 *
 * This logger delegates the logging task to a callback. This level of
 * indirection enables using the default Python logger from C++; or
 * integrating HighFive into some custom logging solution.
 *
 * Using this class directly to log is not intended. Rather you should use
 *   - `HIGHFIVE_LOG_DEBUG{,_IF}`
 *   - `HIGHFIVE_LOG_INFO{,_IF}`
 *   - `HIGHFIVE_LOG_WARNING{,_IF}`
 *   - `HIGHFIVE_LOG_ERROR{,_IF}`
 *
 * This is intended to used as a singleton, via `get_global_logger()`.
 */
class Logger {
  public:
    using callback_type =
        std::function<void(LogSeverity, const std::string&, const std::string&, int)>;

  public:
    Logger() = delete;
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    explicit Logger(callback_type cb)
        : _cb(std::move(cb)) {}

    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    inline void log(LogSeverity severity,
                    const std::string& message,
                    const std::string& file,
                    int line) {
        _cb(severity, message, file, line);
    }

    inline void set_logging_callback(callback_type cb) {
        _cb = std::move(cb);
    }

  private:
    callback_type _cb;
};

inline void default_logging_callback(LogSeverity severity,
                                     const std::string& message,
                                     const std::string& file,
                                     int line) {
    std::clog << file << ": " << line << " :: " << to_string(severity) << message << std::endl;
}

/// \brief Obtain a reference to the logger used by HighFive.
///
/// This uses a Meyers singleton, to ensure that the global logger is
/// initialized with a safe default logger, before it is used.
///
/// Note: You probably don't need to call this function explicitly.
///
inline Logger& get_global_logger() {
    static Logger logger(&default_logging_callback);
    return logger;
}

/// \brief Sets the callback that's used by the logger.
inline void register_logging_callback(Logger::callback_type cb) {
    auto& logger = get_global_logger();
    logger.set_logging_callback(std::move(cb));
}

namespace detail {
/// \brief Log a `message` with severity `severity`.
inline void log(LogSeverity severity,
                const std::string& message,
                const std::string& file,
                int line) {
    auto& logger = get_global_logger();
    logger.log(severity, message, file, line);
}
}  // namespace detail

#if HIGHFIVE_LOG_LEVEL <= HIGHFIVE_LOG_LEVEL_DEBUG
#define HIGHFIVE_LOG_DEBUG(message) \
    ::HighFive::detail::log(::HighFive::LogSeverity::Debug, (message), __FILE__, __LINE__);

// Useful, for the common pattern: if ...; then log something.
#define HIGHFIVE_LOG_DEBUG_IF(cond, message) \
    if ((cond)) {                            \
        HIGHFIVE_LOG_DEBUG((message));       \
    }

#else
#define HIGHFIVE_LOG_DEBUG(message)          ;
#define HIGHFIVE_LOG_DEBUG_IF(cond, message) ;
#endif

#if HIGHFIVE_LOG_LEVEL <= HIGHFIVE_LOG_LEVEL_INFO
#define HIGHFIVE_LOG_INFO(message) \
    ::HighFive::detail::log(::HighFive::LogSeverity::Info, (message), __FILE__, __LINE__);

// Useful, for the common pattern: if ...; then log something.
#define HIGHFIVE_LOG_INFO_IF(cond, message) \
    if ((cond)) {                           \
        HIGHFIVE_LOG_INFO((message));       \
    }

#else
#define HIGHFIVE_LOG_INFO(message)          ;
#define HIGHFIVE_LOG_INFO_IF(cond, message) ;
#endif


#if HIGHFIVE_LOG_LEVEL <= HIGHFIVE_LOG_LEVEL_WARN
#define HIGHFIVE_LOG_WARN(message) \
    ::HighFive::detail::log(::HighFive::LogSeverity::Warn, (message), __FILE__, __LINE__);

// Useful, for the common pattern: if ...; then log something.
#define HIGHFIVE_LOG_WARN_IF(cond, message) \
    if ((cond)) {                           \
        HIGHFIVE_LOG_WARN((message));       \
    }

#else
#define HIGHFIVE_LOG_WARN(message)          ;
#define HIGHFIVE_LOG_WARN_IF(cond, message) ;
#endif

#if HIGHFIVE_LOG_LEVEL <= HIGHFIVE_LOG_LEVEL_ERROR
#define HIGHFIVE_LOG_ERROR(message) \
    ::HighFive::detail::log(::HighFive::LogSeverity::Error, (message), __FILE__, __LINE__);

// Useful, for the common pattern: if ...; then log something.
#define HIGHFIVE_LOG_ERROR_IF(cond, message) \
    if ((cond)) {                            \
        HIGHFIVE_LOG_ERROR((message));       \
    }

#else
#define HIGHFIVE_LOG_ERROR(message)          ;
#define HIGHFIVE_LOG_ERROR_IF(cond, message) ;
#endif

}  // namespace HighFive
