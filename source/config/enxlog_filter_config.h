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

#ifndef ENXLOG_FILTER_CONFIG_H
#define ENXLOG_FILTER_CONFIG_H

#include <enx/log/enxlog.h>

__BEGIN_DECLS

/**
 * @brief Filter configuration
 */
struct enxlog_filter_config
{
    struct enxlog_filter_config_entry *root;
};

/**
 * @brief Filter configuration entry
 */
struct enxlog_filter_config_entry
{
    char* path;
    enum enxlog_loglevel loglevel;
    struct enxlog_filter_config_entry* child;
    struct enxlog_filter_config_entry* next;
};

/**
 * @brief Creates a filter configuration
 */
struct enxlog_filter_config *enxlog_filter_config_create();

/**
 * @brief Destroys a filter configuration
 */
void enxlog_filter_config_destroy(struct enxlog_filter_config *obj);

/**
 * @brief Appends an entry to the filter configuration
 */
void enxlog_filter_config_append(
    struct enxlog_filter_config *obj,
    const char *path,
    enum enxlog_loglevel loglevel,
    enum enxlog_loglevel default_loglevel);

/**
 * @brief Transforms the configuration to a filter
 */
struct enxlog_filter *enxlog_filter_config_transform(
    const struct enxlog_filter_config *obj);

/**
 * @brief Destroys a transformed filter
 */
void enxlog_filter_config_transform_destroy(
     struct enxlog_filter *obj);


__END_DECLS

#endif
