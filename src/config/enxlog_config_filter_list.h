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

#ifndef ENXLOG_CONFIG_FILTER_LIST_H
#define ENXLOG_CONFIG_FILTER_LIST_H

#include <enx/log/enxlog.h>

__BEGIN_DECLS


struct enxlog_filter_entry *enxlog_config_filter_list_create();
void enxlog_config_filter_list_destroy(const struct enxlog_filter_entry *filter_list);
struct enxlog_filter_entry *enxlog_config_filter_list_find_entry(const struct enxlog_filter_entry *parent, const char *path);
struct enxlog_filter_entry* enxlog_config_filter_list_append_entry(struct enxlog_filter_entry *parent, const char *path, enum enxlog_loglevel loglevel);

__END_DECLS

#endif
