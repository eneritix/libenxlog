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
 *
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
 *
 */
struct enxlog_logger
{
    const char **path;
};

/**
 * Declare a logger
 *
 */
#define LOGGER_DECLARE(name, ...) \
static const struct enxlog_logger* name = (const struct enxlog_logger []) { \
    { \
        .path = (const char* []) { \
        __VA_ARGS__ \
        ,0 \
        } \
    } \
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
    char *path;
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
 * @param _name The variable name of the filter
 */
#define enxlog_filter(_name)                                \
const struct enxlog_filter* _name =                         \
    (const struct enxlog_filter*)                           \
    (const struct enxlog_filter []) {                       \
    {                                                       \
    .entries = (struct enxlog_filter_entry*)                \
               (const struct enxlog_filter_entry []) {

/**
 * Ends a filter definition
 */
#define enxlog_end_filter()                                 \
            { .path = 0 }                                   \
            }                                               \
        }                                                   \
    };

/**
 * Starts a filter entry
 * @param _path The variable name of the filter
 */
#define enxlog_filter_entry(_path, _loglevel)               \
    {                                                       \
        .path = _path,                                      \
        .loglevel = _loglevel,                              \
        .children = (struct enxlog_filter_entry*)           \
            (const struct enxlog_filter_entry []) {

/**
 * End a filter entry
 */
#define enxlog_end_filter_entry()                           \
            { .path = 0 }                                   \
        }                                                   \
    },

/** @} */


/** \defgroup sink_functions Sink Functions
 * @{
 */

/**
 * Sink
 *
 */
struct enxlog_sink
{
    bool valid;
    void *context;

    bool (*fn_init)(void *context);
    void (*fn_shutdown)(void *context);

    void (*fn_log_entry_open)(
        void* context,
        const struct enxlog_logger *logger,
        enum enxlog_loglevel loglevel,
        const char *func,
        unsigned int line);

    void (*fn_log_entry_write)(
        void* context,
        const char *ptr,
        size_t length);

    void (*fn_log_entry_close)(void *context);
};

/**
 * Start a sink list
 *
 */
#define enxlog_sink_list(_name)                             \
const struct enxlog_sink *_name =                           \
    (const struct enxlog_sink*)                             \
    (const struct enxlog_sink []) {

/**
 * End a sink list
 *
 */
#define enxlog_end_sink_list()                              \
    { .valid = false }                                      \
};

/**
 * Declare a sink
 *
 */
#define enxlog_sink(_context, _fn_init, _fn_shutdown, _fn_log_entry_open, _fn_log_entry_write, _fn_log_entry_close)                                    \
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
 * Lock
 *
 */
struct enxlog_lock
{
    void (*fn_lock)(void *context);
    void (*fn_unlock)(void *context);
    void* context;
};


/**
 * Declare a lock
 *
 */
#define enxlog_lock(_name, _context, _fn_lock, _fn_unlock)  \
const struct enxlog_lock *_name =                           \
    (const struct enxlog_lock*)                             \
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
 * @param filter_list The filter list
 */
bool enxlog_init(
    enum enxlog_loglevel default_loglevel,
    const struct enxlog_sink *sinks,
    const struct enxlog_lock *lock,
    const struct enxlog_filter *filter);

/**
 * Shuts down the logging library
 *
 */
void enxlog_shutdown(void);

/**
 * Returns true when output is allowed for the given logger and loglevel
 * @private
 */
bool enxlog_allow_output(
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel);

/**
 * Opens a log entry
 * @private
 */
void enxlog_log_entry_open(
    const struct enxlog_logger* logger,
    enum enxlog_loglevel loglevel,
    const char* func,
    unsigned int line);

/**
 * Writes to a log entry
 * @private
 */
bool enxlog_log_entry_write(
    void *context,
    const char *ptr,
    size_t length);

/**
 * Closes a log entry
 * @private
 */
void enxlog_log_entry_close(void);

/**
 * Called by the logging macros
 * @private
 */
void enxlog_log(
    const struct enxlog_logger* logger,
    enum enxlog_loglevel loglevel,
    const char* func,
    unsigned int line,
    const char* format,
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
 * @param args A variable list of arguments
 */
#define LOG_ERROR(logger, format, ...)                                                  \
do {                                                                                    \
    const struct enxtxt_fstr_arg* __args = (const struct enxtxt_fstr_arg []) {          \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_ERROR, __FUNCTION__, __LINE__, format, __args);         \
} while (0)

#define LOG_WARN(logger, format, ...)                                                   \
do {                                                                                    \
    const struct enxtxt_fstr_arg* __args = (const struct enxtxt_fstr_arg []) {          \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_WARN, __FUNCTION__, __LINE__, format, __args);          \
} while (0)

#define LOG_INFO(logger, format, ...)                                                   \
do {                                                                                    \
    const struct enxtxt_fstr_arg* __args = (const struct enxtxt_fstr_arg []) {          \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_INFO, __FUNCTION__, __LINE__, format, __args);          \
} while (0)

#define LOG_DEBUG(logger, format, ...)                                                  \
do {                                                                                    \
    const struct enxtxt_fstr_arg* __args = (const struct enxtxt_fstr_arg []) {          \
    __VA_ARGS__                                                                         \
    };                                                                                  \
    enxlog_log(logger, LOGLEVEL_DEBUG, __FUNCTION__, __LINE__, format, __args);         \
} while (0)

/** @} */


__END_DECLS

#endif
