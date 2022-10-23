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

#include <enx/log/enxlog.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


static enum enxlog_loglevel enxlog_default_loglevel = LOGLEVEL_NONE;
static const struct enxlog_sink* enxlog_sinks = NULL;
static const struct enxlog_lock *enxlog_lock = NULL;
static const struct enxlog_filter *enxlog_filter = NULL;

void enxlog_init(
    enum enxlog_loglevel default_loglevel,
    const struct enxlog_sink *sinks,
    const struct enxlog_lock *lock,
    const struct enxlog_filter *filter)
{
    enxlog_default_loglevel = default_loglevel;
    enxlog_sinks = sinks;
    enxlog_lock = lock;
    enxlog_filter = filter;

}

void enxlog_log(
    const struct enxlog_logger* logger,
    enum enxlog_loglevel loglevel,
    const char* func,
    unsigned int line,
    const char* fmt, ...)
{
    // No sinks
    if (enxlog_sinks == NULL) {
        return;
    }

    // Lock
    if (enxlog_lock) {
        enxlog_lock->fn_lock(enxlog_lock->context);
    }

    enum enxlog_loglevel config_loglevel = enxlog_default_loglevel;

    const struct enxlog_filter_entry* filter_entry = enxlog_filter->entries;
    const char** path = logger->path;

    while (*path) {

        while (filter_entry->path) {
            if (strcmp(filter_entry->path, *path) == 0) {
                config_loglevel = filter_entry->loglevel;
                filter_entry = filter_entry->children;
                path++;
                break;
            } else {
                filter_entry++;
            }
        }

        if (filter_entry->path == 0) {
            break;
        }
    }

    if (loglevel > config_loglevel) {
        if (enxlog_lock) {
            enxlog_lock->fn_unlock(enxlog_lock->context);
        }
        return;
    }

    // Log
    const struct enxlog_sink* sink = enxlog_sinks;
    while (sink->fn_output) {
        va_list args;
        va_start(args, fmt);
        sink->fn_output(sink->context, logger, loglevel, func, line, fmt, args);
        va_end(args);
        sink++;
    }

    if (enxlog_lock) {
        enxlog_lock->fn_unlock(enxlog_lock->context);
    }
}
