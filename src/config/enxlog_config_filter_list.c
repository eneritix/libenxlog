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

#include "enxlog_config_filter_list.h"

#include <stdlib.h>
#include <string.h>


static struct enxlog_filter_entry *enxlog_config_filter_list_create_empty_entry();
static void enxlog_config_filter_list_destroy_recursively(const struct enxlog_filter_entry* filter_list);


struct enxlog_filter_entry *enxlog_config_filter_list_create()
{
    struct enxlog_filter_entry *result = (struct enxlog_filter_entry *)malloc(sizeof(struct enxlog_filter_entry));
    result->loglevel = LOGLEVEL_NONE;
    result->path = strdup("");
    result->children = enxlog_config_filter_list_create_empty_entry();

    return result;
}

void enxlog_config_filter_list_destroy(const struct enxlog_filter_entry* filter_list)
{
    enxlog_config_filter_list_destroy_recursively(filter_list);
    free((void *)filter_list);
}

struct enxlog_filter_entry* enxlog_config_filter_list_find_entry(const struct enxlog_filter_entry* parent, const char* path)
{
    const struct enxlog_filter_entry* search = parent->children;
    while ((search->path) && (strcmp(search->path, path) != 0)) {
        search++;
    }

    if (search->path == NULL) {
        search = NULL;
    }

    return (struct enxlog_filter_entry*)search;
}

struct enxlog_filter_entry* enxlog_config_filter_list_append_entry(struct enxlog_filter_entry* parent, const char* path, enum enxlog_loglevel loglevel)
{
    const struct enxlog_filter_entry* search = parent->children;
    while (search->path) {
        search++;
    }

    size_t entries = (search - parent->children) + 1;
    parent->children = (struct enxlog_filter_entry*)realloc((void*)parent->children, (entries + 1) * sizeof(struct enxlog_filter_entry));

    memcpy((void*)(parent->children + entries), parent->children + entries - 1, sizeof(struct enxlog_filter_entry));

    struct enxlog_filter_entry* child = (struct enxlog_filter_entry*)(parent->children + (entries - 1));
    child->path = path;
    child->loglevel = loglevel;
    child->children = enxlog_config_filter_list_create_empty_entry();

    return child;
}

static struct enxlog_filter_entry* enxlog_config_filter_list_create_empty_entry()
{
    struct enxlog_filter_entry* node = malloc(sizeof(struct enxlog_filter_entry));
    node->loglevel = LOGLEVEL_NONE;
    node->path = NULL;
    node->children = NULL;

    return node;
}

static void enxlog_config_filter_list_destroy_recursively(const struct enxlog_filter_entry* filter_list)
{
    // Recursively deallocate the children
    const struct enxlog_filter_entry* index = filter_list->children;
    while (index->path) {
        enxlog_config_filter_list_destroy_recursively(index);
        index++;
    }

    // Free the object pointers
    free((char *)filter_list->path);
    free((void *)filter_list->children);
}
