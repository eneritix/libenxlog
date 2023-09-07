/*
    Copyright (c) 2022 Eneritix (Pty) Ltd

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#ifndef ENXLOG_H
#define ENXLOG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <enx/txt/fstr.h>


__BEGIN_DECLS

/**
 * Loglevel enumeration
 */
enum enxlog_loglevel
{
    LOGLEVEL_NONE = 0,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARN,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG
};

/** \defgroup logger_functions Logger Functions
 * @{
 */

/**
 * Logger
 */
struct enxlog_logger
{
    const char **name;
};

/**
 * Define a logger
 * @param _var_name The variable name of the logger
 * @param ... The name of the logger, specified as a list of comma separated parts
 */
#define LOGGER(_var_name, ...)                              \
static const struct enxlog_logger *_var_name =              \
    (const struct enxlog_logger []) {                       \
    {                                                       \
        .name = (const char* []) {                          \
        __VA_ARGS__                                         \
        ,0                                                  \
        }                                                   \
    }                                                       \
}

/** @} */

/** \defgroup filter_functions Filter Functions
 * @{
 */

/**
 * Filter entry
 */
struct enxlog_filter_entry
{
    char *name_part;
    enum enxlog_loglevel loglevel;
    struct enxlog_filter_entry *children;
};

/**
 * Filter
 */
struct enxlog_filter
{
    struct enxlog_filter_entry *entries;
};

/**
 * Starts a filter definition
 * @param _var_name The variable name of the filter
 */
#define enxlog_filter(_var_name)                            \
static const struct enxlog_filter *_var_name =              \
    (const struct enxlog_filter []) {                       \
    {                                                       \
    .entries = (struct enxlog_filter_entry *)               \
               (const struct enxlog_filter_entry []) {

/**
 * Ends a filter definition
 */
#define enxlog_end_filter()                                 \
            { .name_part = 0 }                              \
            }                                               \
        }                                                   \
    };

/**
 * Starts a filter entry
 * @param _name_part The name part of the filter entry
 * @param _loglevel The loglevel of the filter up to this point
 */
#define enxlog_filter_entry(_name_part, _loglevel)          \
    {                                                       \
        .name_part = _name_part,                            \
        .loglevel = _loglevel,                              \
        .children = (struct enxlog_filter_entry *)          \
            (const struct enxlog_filter_entry []) {

/**
 * Ends a filter entry
 */
#define enxlog_end_filter_entry()                           \
            { .name_part = 0 }                              \
        }                                                   \
    },

/** @} */


/** \defgroup sink_functions Sink Functions
 * @{
 */

/**
 * @brief Sink init callback function
 * @param context The user supplied context
 */
typedef bool (*enxlog_sink_init_fn_t)(void *context);

/**
 * @brief Sink shutdown callback function
 * @param context The user supplied context
 */
typedef void (*enxlog_sink_shutdown_fn_t)(void *context);

/**
 * @brief Sink log entry open callback function
 * @param context The user supplied context
 * @param logger The logger that the log macro was invoked on
 * @param loglevel The log level of the invoked log macro
 * @param func The function that the log macro was invoked in
 * @param line The line that the log macro was invoked on
 */
typedef void (*enxlog_sink_log_entry_open_fn_t)(
    void *context,
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel,
    const char *func,
    unsigned int line);

/**
 * @brief Sink log entry write callback function
 * @param context The user supplied context
 * @param ptr The data to write (not null-terminated)
 * @param length the length of the data
 */
typedef void (*enxlog_sink_log_entry_write_fn_t)(
    void* context,
    const char *ptr,
    size_t length);

/**
 * @brief Sink log entry close callback function
 * @param context The user supplied context
 */
typedef void (*enxlog_sink_log_entry_close_fn_t)(void *context);

/**
 * Sink
 */
struct enxlog_sink
{
    bool valid;
    void *context;
    enxlog_sink_init_fn_t fn_init;
    enxlog_sink_shutdown_fn_t fn_shutdown;
    enxlog_sink_log_entry_open_fn_t fn_log_entry_open;
    enxlog_sink_log_entry_write_fn_t fn_log_entry_write;
    enxlog_sink_log_entry_close_fn_t fn_log_entry_close;
};

/**
 * Starts a sink list
 * @param _var_name The variable name of the sink list
 */
#define enxlog_sink_list(_var_name)                         \
static const struct enxlog_sink *_var_name =                \
    (const struct enxlog_sink *)                            \
    (const struct enxlog_sink []) {

/**
 * Ends a sink list
 */
#define enxlog_end_sink_list()                              \
    { .valid = false }                                      \
};

/**
 * Declares a sink
 * @param _context The user supplied context
 * @param _fn_init The sink initialization function. See #enxlog_sink_init_fn_t
 * @param _fn_shutdown The sink shutdown function. See #enxlog_sink_shutdown_fn_t
 * @param _fn_log_entry_open The log entry open function. See #enxlog_sink_log_entry_open_fn_t
 * @param _fn_log_entry_write The log entry write function. See #enxlog_sink_log_entry_write_fn_t
 * @param _fn_log_entry_close The log entry close function. See #enxlog_sink_log_entry_close_fn_t
 */
#define enxlog_sink(_context, _fn_init, _fn_shutdown, _fn_log_entry_open, _fn_log_entry_write, _fn_log_entry_close) \
    {                                                       \
        .valid = true,                                      \
        .context = _context,                                \
        .fn_init = _fn_init,                                \
        .fn_shutdown = _fn_shutdown,                        \
        .fn_log_entry_open = _fn_log_entry_open,            \
        .fn_log_entry_write = _fn_log_entry_write,          \
        .fn_log_entry_close = _fn_log_entry_close           \
    },

/** @} */

/** \defgroup lock_functions Lock Functions
 * @{
 */

/**
 * @brief Lock lock function
 * @param context The user supplied context
 */
typedef void (*enxlog_lock_lock_fn_t)(void *context);

/**
 * @brief Lock unlock function
 * @param context The user supplied context
 */
typedef void (*enxlog_lock_unlock_fn_t)(void *context);

/**
 * Lock
 */
struct enxlog_lock
{
    enxlog_lock_lock_fn_t fn_lock;
    enxlog_lock_unlock_fn_t fn_unlock;
    void *context;
};


/**
 * Define a lock
 * @param _var_name The variable name of the lock
 * @param _context The user supplied context
 * @param _fn_lock The lock function. See #enxlog_lock_lock_fn_t
 * @param _fn_unlock The unlock function. See #enxlog_lock_unlock_fn_t
 */
#define enxlog_lock(_var_name, _context, _fn_lock, _fn_unlock)  \
static const struct enxlog_lock *_var_name =                \
    (const struct enxlog_lock []) {                         \
    {                                                       \
        .context = _context,                                \
        .fn_lock = _fn_lock,                                \
        .fn_unlock = _fn_unlock                             \
    }                                                       \
};

/** @} */


/** \defgroup general_functions General Functions
 *  @{
 */


/**
 * Initialize the logging library
 *
 * @param default_loglevel The default loglevel
 * @param sinks A list of sinks
 * @param lock The lock to use, or NULL if locking is not required
 * @param filter The filter
 */
bool enxlog_init(
    enum enxlog_loglevel default_loglevel,
    const struct enxlog_sink *sinks,
    const struct enxlog_lock *lock,
    const struct enxlog_filter *filter);

/**
 * Shuts down the logging library
 */
void enxlog_shutdown(void);

/**
 * Called by the logging macros
 * @private
 */
void enxlog_log(
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel,
    const char *func,
    unsigned int line,
    const char *format,
    const struct enxtxt_fstr_arg *args);

/** @} */


/** \defgroup logging_macros Logging macros
 *  @{
 */

/* Log macros */

/**
 * Logs an error
 * @param logger The logger
 * @param format A format string
 * @param ... A variable list of arguments
 */
#define LOG_ERROR(logger, format, ...)                                                  \
do {                                                                                    \
    const struct enxtxt_fstr_arg __args[] = {                                           \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_ERROR, __FUNCTION__, __LINE__, format, __args);         \
} while (0)

/**
 * Logs a warning
 * @param logger The logger
 * @param format A format string
 * @param ... A variable list of arguments
 */
#define LOG_WARN(logger, format, ...)                                                   \
do {                                                                                    \
    const struct enxtxt_fstr_arg __args[] = {                                           \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_WARN, __FUNCTION__, __LINE__, format, __args);          \
} while (0)

/**
 * Logs information
 * @param logger The logger
 * @param format A format string
 * @param ... A variable list of arguments
 */
#define LOG_INFO(logger, format, ...)                                                   \
do {                                                                                    \
    const struct enxtxt_fstr_arg __args[] = {                                           \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_INFO, __FUNCTION__, __LINE__, format, __args);          \
} while (0)

/**
 * Logs debug information
 * @param logger The logger
 * @param format A format string
 * @param ... A variable list of arguments
 */
#define LOG_DEBUG(logger, format, ...)                                                  \
do {                                                                                    \
    const struct enxtxt_fstr_arg __args[] = {                                           \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_DEBUG, __FUNCTION__, __LINE__, format, __args);         \
} while (0)

/** @} */


__END_DECLS

#endif
