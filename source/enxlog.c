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
#include <enx/txt/format.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


static enum enxlog_loglevel enxlog_default_loglevel = LOGLEVEL_NONE;
static const struct enxlog_sink* enxlog_sinks = NULL;
static const struct enxlog_lock *enxlog_lock = NULL;
static const struct enxlog_filter *enxlog_filter = NULL;

bool enxlog_init(
    enum enxlog_loglevel default_loglevel,
    const struct enxlog_sink *sinks,
    const struct enxlog_lock *lock,
    const struct enxlog_filter *filter)
{
    bool result = true;

    enxlog_default_loglevel = default_loglevel;
    enxlog_sinks = sinks;
    enxlog_lock = lock;
    enxlog_filter = filter;

    const struct enxlog_sink* sink = enxlog_sinks;
    while (sink->valid) {
        if (sink->fn_init) {
            result &= sink->fn_init(sink->context);
        }
        sink++;
    }

    return result;
}

void enxlog_shutdown(void)
{
    const struct enxlog_sink* sink = enxlog_sinks;
    while (sink->valid) {
        if (sink->fn_shutdown) {
            sink->fn_shutdown(sink->context);
        }

        sink++;
    }
}

bool enxlog_allow_output(
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel)
{
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

    return (loglevel <= config_loglevel);
}

void enxlog_log_entry_open(
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel,
    const char *func,
    unsigned int line)
{
    // No sinks
    if (enxlog_sinks == NULL) {
        return;
    }

    // Lock
    if (enxlog_lock) {
        enxlog_lock->fn_lock(enxlog_lock->context);
    }

    // Log
    const struct enxlog_sink* sink = enxlog_sinks;
    while (sink->valid) {
        if (sink->fn_log_entry_open) {
            sink->fn_log_entry_open(sink->context, logger, loglevel, func, line);
        }
        sink++;
    }
}

void enxlog_log_entry_write(const char *ptr, size_t length)
{
    // Log
    const struct enxlog_sink* sink = enxlog_sinks;
    while (sink->valid) {
        if (sink->fn_log_entry_write) {
            sink->fn_log_entry_write(sink->context, ptr, length);
        }
        sink++;
    }
}

void enxlog_log_entry_close(void)
{
    // Log
    const struct enxlog_sink* sink = enxlog_sinks;
    while (sink->valid) {
        if (sink->fn_log_entry_close) {
            sink->fn_log_entry_close(sink->context);
        }
        sink++;
    }

    if (enxlog_lock) {
        enxlog_lock->fn_unlock(enxlog_lock->context);
    }
}

void enxlog_log(
    const struct enxlog_logger* logger,
    enum enxlog_loglevel loglevel,
    const char* func,
    unsigned int line,
    const char* format,
    const struct enxlog_fmt_arg *args)
{
    if (enxlog_allow_output(logger, loglevel)) {

        enxlog_log_entry_open(logger, loglevel, func, line);

        do {
            const char *ptr = format;
            while ((*ptr != '{') && (*ptr != 0)) {
                ptr++;
            }

            enxlog_log_entry_write(format, ptr - format);

            if (*ptr == '{') {
                while ((*ptr != '}') && (*ptr != 0)) {
                    ptr++;
                }

                if (*ptr == '}') {
                    if (args->fn_fmt) {
                        args->fn_fmt(args);
                    }
                    args++;
                    ptr++;
                }
            }

            format = ptr;

        } while (*format != 0);


        enxlog_log_entry_close();
    }
}


void enxlog_fmt_int(const struct enxlog_fmt_arg *arg)
{
    struct enxtxt_fmt_result result = enxtxt_fmt_s32_dec(arg->value._int);
    enxlog_log_entry_write(result.str, result.length);
}

void enxlog_fmt_uint(const struct enxlog_fmt_arg *arg)
{
    struct enxtxt_fmt_result result = enxtxt_fmt_u32_dec(arg->value._uint);
    enxlog_log_entry_write(result.str, result.length);
}

void enxlog_fmt_h8(const struct enxlog_fmt_arg *arg)
{
    struct enxtxt_fmt_result result = enxtxt_fmt_u8_hex(arg->value._uint);
    enxlog_log_entry_write(result.str, result.length);
}

void enxlog_fmt_h16(const struct enxlog_fmt_arg *arg)
{
    struct enxtxt_fmt_result result = enxtxt_fmt_u16_hex(arg->value._uint);
    enxlog_log_entry_write(result.str, result.length);
}

void enxlog_fmt_h32(const struct enxlog_fmt_arg *arg)
{
    struct enxtxt_fmt_result result = enxtxt_fmt_u32_hex(arg->value._uint);
    enxlog_log_entry_write(result.str, result.length);
}

void enxlog_fmt_str(const struct enxlog_fmt_arg *arg)
{
    enxlog_log_entry_write(arg->value._str, strlen(arg->value._str));
}

void enxlog_fmt_h8_array(const struct enxlog_fmt_arg *arg)
{
    const struct enxlog_fmt_h8_array_metadata *data = (const struct enxlog_fmt_h8_array_metadata *)arg->value._user;
    char format_buffer[32];

    const unsigned char *ptr = data->ptr;
    size_t length = data->length;

    while (length) {
        struct enxtxt_fmt_array_result result = enxtxt_fmt_u8_hex_array(ptr, length, " ", format_buffer, sizeof(format_buffer));
        enxlog_log_entry_write(format_buffer, result.bytes_written);
        ptr += result.entries_processed;
        length -= result.entries_processed;
    }
}
