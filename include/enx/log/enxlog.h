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


#include <stdarg.h>
#include <sys/cdefs.h>


__BEGIN_DECLS

/**
 * @brief Loglevel enumeration
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

/**
 * @brief Logger
 *
 */
struct enxlog_logger
{
    const char** path;
};

/**
 * @brief Filter entry
 *
 */
struct enxlog_filter_entry
{
    const char* path;
    enum enxlog_loglevel loglevel;
    const struct enxlog_filter_entry* children;
};

/**
 * @brief Sink
 *
 */
struct enxlog_sink
{
    void (*fn_output)(
        void* context,
        const struct enxlog_logger* logger,
        enum enxlog_loglevel loglevel,
        const char* func,
        unsigned int line,
        const char* fmt,
        va_list ap);

    void (*fn_shutdown)(void* context);

    void* context;
};

/**
 * @brief Lock
 *
 */
struct enxlog_lock
{
    void (*fn_lock)(void* context);
    void (*fn_unlock)(void* context);
    void* context;
};



/**
 * @brief Start a filter tree
 *
 */
#define enxlog_filter_tree(_name)                           \
const struct enxlog_filter_entry* _name =                   \
    (const struct enxlog_filter_entry*)                     \
    (const struct enxlog_filter_entry []) {


/**
 * @brief End a filter tree
 *
 */
#define enxlog_end_filter_tree()                            \
        { .path = 0 }                                       \
    };

/**
 * @brief Start a filter entry
 *
 */
#define enxlog_filter(_path, _loglevel)                     \
    {                                                       \
        .path = _path,                                      \
        .loglevel = _loglevel,                              \
        .children = (struct enxlog_filter_entry*)           \
            (const struct enxlog_filter_entry []) {

/**
 * @brief End a filter entry
 *
 */
#define enxlog_endfilter()                                  \
            { .path = 0 }                                   \
        }                                                   \
    },

/**
 * @brief Start a sink list
 *
 */
#define enxlog_sink_list(_name)                             \
const struct enxlog_sink *_name =                           \
    (const struct enxlog_sink*)                             \
    (const struct enxlog_sink []) {

/**
 * @brief End a sink list
 *
 */
#define enxlog_end_sink_list()                              \
    { .fn_output = 0 }                                      \
};

/**
 * @brief Declare a sink
 *
 */
#define enxlog_sink(_context, _fn_output, _fn_shutdown)     \
    {                                                       \
        .fn_output = _fn_output,                            \
        .fn_shutdown = _fn_shutdown,                        \
        .context = _context                                 \
    },

/**
 * @brief Declare a lock
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

/**
 * @brief Declare a logger
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


/**
 * @brief Initialize the logging library
 *
 * @param default_loglevel The default loglevel
 * @param sinks A list of sinks
 * @param lock The lock to use, or NULL if locking is not required
 * @param filter_list The filter list
 */
void enxlog_init(
    enum enxlog_loglevel default_loglevel,
    const struct enxlog_sink *sinks,
    const struct enxlog_lock *lock,
    const struct enxlog_filter_entry *filter_list);

/**
 * @brief Log function
 *
 * This function is called by the log macros and should not be called directly
 */
void enxlog_log(
        const struct enxlog_logger* logger,
        enum enxlog_loglevel loglevel,
        const char* func,
        unsigned int line,
        const char* fmt, ...);


#define LOG_ERROR(logger, fmt, ...) do { enxlog_log(logger, LOGLEVEL_ERROR, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define LOG_WARN(logger, fmt, ...) do { enxlog_log(logger, LOGLEVEL_WARN, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define LOG_INFO(logger, fmt, ...) do { enxlog_log(logger, LOGLEVEL_INFO, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define LOG_DEBUG(logger, fmt, ...) do { enxlog_log(logger, LOGLEVEL_DEBUG, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)


__END_DECLS;

#endif
