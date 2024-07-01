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

#ifndef ENXLOG_SINK_FILE_H
#define ENXLOG_SINK_FILE_H

#include <enx/log/enxlog.h>

#include <stdio.h>
#include <sys/cdefs.h>

__BEGIN_DECLS


struct enxlog_sink_file_context
{
    const char *path;
    FILE *file;
    size_t tag_length;
};


struct enxlog_sink_file_context *enxlog_sink_file_create();
void enxlog_sink_file_destroy(void *context);

bool enxlog_sink_file_init(void *context);
void enxlog_sink_file_shutdown(void *context);

void enxlog_sink_file_log_entry_open(
    void *context,
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel,
    const char *func,
    unsigned int line);

void enxlog_sink_file_log_entry_write(
    void *context,
    const char *ptr,
    size_t length);

void enxlog_sink_file_log_entry_close(
    void *context);


__END_DECLS

#endif
