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

#ifndef ENXLOG_CONFIG_FILTER_TREE_H
#define ENXLOG_CONFIG_FILTER_TREE_H

#include <enx/log/enxlog.h>

__BEGIN_DECLS

/**
 * @brief Creates a filter tree
 *
 */
struct enxlog_filter_entry *enxlog_config_filter_tree_create();

/**
 * @brief Destroys a filter tree
 *
 */
void enxlog_config_filter_tree_destroy(const struct enxlog_filter_entry *root);

/**
 * @brief Finds a child entry
 *
 */
struct enxlog_filter_entry *enxlog_config_filter_tree_find_child(
    const struct enxlog_filter_entry *parent,
    const char *path);

/**
 * @brief Appends a child entry to the parent
 *
 */
struct enxlog_filter_entry* enxlog_config_filter_tree_append_child(
    struct enxlog_filter_entry *parent,
    const char *path,
    enum enxlog_loglevel loglevel);

/**
 * @brief Appends an entry to the parent, creating missing child nodes
 *
 */
struct enxlog_filter_entry* enxlog_config_filter_tree_append(
    struct enxlog_filter_entry *parent,
    const char *path,
    enum enxlog_loglevel loglevel);


__END_DECLS

#endif
