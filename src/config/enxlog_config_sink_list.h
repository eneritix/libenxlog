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

#ifndef ENXLOG_CONFIG_SINK_LIST_H
#define ENXLOG_CONFIG_SINK_LIST_H

#include <stdlib.h>
#include <sys/cdefs.h>
#include <enx/log/enxlog.h>

__BEGIN_DECLS


/**
 * @brief Sink list
 *
 */
struct enxlog_config_sink_list
{
    struct enxlog_sink *sinks;
    size_t sink_count;
};

/**
 * @brief Creates a sink list
 *
 */
struct enxlog_config_sink_list *enxlog_config_sink_list_create();

/**
 * @brief Destroys a sink list
 *
 */
void enxlog_config_sink_list_destroy(struct enxlog_config_sink_list *sink_list);

/**
 * @brief Appends a new empty sink item to the sink list
 *
 */
struct enxlog_sink *enxlog_config_sink_list_append(struct enxlog_config_sink_list *sink_list);


__END_DECLS

#endif
