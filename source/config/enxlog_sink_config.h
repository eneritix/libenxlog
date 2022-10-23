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

#ifndef ENXLOG_SINK_CONFIG_H
#define ENXLOG_SINK_CONFIG_H

#include <stdlib.h>
#include <sys/cdefs.h>
#include <enx/log/config/enxlog_config_parser.h>

__BEGIN_DECLS



/**
 * @brief Sink configuration entry
 */
struct enxlog_sink_config_entry
{
    struct enxlog_sink_parameters *parameters;
    struct enxlog_sink_config_entry *next;
};

/**
 * @brief Sink configuration
 */
struct enxlog_sink_config
{
    struct enxlog_sink_config_entry *sinks;
};

/**
 * @brief Creates a sink configuration
 */
struct enxlog_sink_config *enxlog_sink_config_create();

/**
 * @brief Destroys a sink configuration
 *
 */
void enxlog_sink_config_destroy(struct enxlog_sink_config *obj);

/**
 * @brief Appends an item to the sink list
 *
 */
void enxlog_sink_config_append(
    struct enxlog_sink_config *obj,
    struct enxlog_sink_parameters *parameters);

/**
 * @brief Transforms the configuration to a sink list
 */
struct enxlog_sink *enxlog_sink_config_transform(
    const struct enxlog_sink_config *obj,
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback,
    enxlog_config_parser_error_callback_t error_callback);

/**
 * @brief Destroys a transformed sink_list
 */
void enxlog_sink_config_transform_destroy(
     struct enxlog_sink *obj);


__END_DECLS

#endif
