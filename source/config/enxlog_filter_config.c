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

#include "enxlog_filter_config.h"

#include <enx/txt/tokenizer.h>

#include <stdlib.h>
#include <string.h>


/**
 * @brief Creates a configuration entry
 */
static struct enxlog_filter_config_entry* enxlog_filter_config_entry_create(
    const char *path,
    enum enxlog_loglevel loglevel);

/**
 * @brief Recursively destroys a configuration entry
 */
static void enxlog_filter_config_entry_destroy(struct enxlog_filter_config_entry *obj);

/**
 * @brief Appends a child item to a configuration entry
 * If no child item with the specified path exists the function will add a new
 * child item to the parent, else it will return a pointer to the existing item.
 */
static struct enxlog_filter_config_entry *enxlog_filter_config_entry_append_child(
    struct enxlog_filter_config_entry *parent,
    const char *path,
    enum enxlog_loglevel loglevel);

static void enxlog_filter_config_transform_recursively(
    struct enxlog_filter_config_entry *config_entries,
    struct enxlog_filter_entry **filter_entries);

static void enxlog_filter_config_transform_destroy_recursively(
    struct enxlog_filter_entry *filter_entries);


struct enxlog_filter_config *enxlog_filter_config_create()
{
    struct enxlog_filter_config *obj = (struct enxlog_filter_config *)malloc(sizeof(struct enxlog_filter_config));
    obj->root = enxlog_filter_config_entry_create(NULL, LOGLEVEL_NONE);

    return obj;
}

void enxlog_filter_config_destroy(struct enxlog_filter_config *obj)
{
    enxlog_filter_config_entry_destroy(obj->root);
    free(obj);
}

void enxlog_filter_config_append(
    struct enxlog_filter_config *obj,
    const char *path,
    enum enxlog_loglevel loglevel)
{
    struct enxlog_filter_config_entry *entry = obj->root;
    struct enxtxt_tokenizer tokenizer;
    struct enxtxt_token token;

    enxtxt_tokenizer_init(&tokenizer, path, 0);

    while (enxtxt_tokenizer_get_next(&tokenizer, '.', &token)) {
        char *path = strndup(token.ptr, token.length);
        entry = enxlog_filter_config_entry_append_child(
            entry,
            path,
            token.final ? loglevel : LOGLEVEL_NONE);
        free(path);
    }
}

static struct enxlog_filter_config_entry* enxlog_filter_config_entry_create(
    const char *path,
    enum enxlog_loglevel loglevel)
{
    struct enxlog_filter_config_entry *obj = (struct enxlog_filter_config_entry *)malloc(sizeof(struct enxlog_filter_config_entry));
    obj->path = path ? strdup(path) : NULL;
    obj->loglevel = loglevel;
    obj->child = NULL;
    obj->next = NULL;

    return obj;
}

static void enxlog_filter_config_entry_destroy(struct enxlog_filter_config_entry *obj)
{
    if (obj->path) {
        free(obj->path);
    }

    if (obj->next) {
        enxlog_filter_config_entry_destroy(obj->next);
    }

    if (obj->child) {
        enxlog_filter_config_entry_destroy(obj->child);
    }

    free(obj);
}

static struct enxlog_filter_config_entry *enxlog_filter_config_entry_append_child(
    struct enxlog_filter_config_entry *parent,
    const char *path,
    enum enxlog_loglevel loglevel)
{
    struct enxlog_filter_config_entry *result = NULL;

    // No children
    if (parent->child == NULL) {
        parent->child = enxlog_filter_config_entry_create(path, loglevel);
        result = parent->child;

    // Has children
    } else {
        struct enxlog_filter_config_entry *search = parent->child;
        bool found = false;
        do {
            found = (strcmp(search->path, path) == 0);
            if (!found && search->next) {
                search = search->next;
            }
        } while (!found && search->next);

        if (found) {
            result = search;

        } else {
            search->next = enxlog_filter_config_entry_create(path, loglevel);
            result = search->next;
        }
    }

    return result;
}

struct enxlog_filter *enxlog_filter_config_transform(
    const struct enxlog_filter_config *obj)
{
    struct enxlog_filter *result = (struct enxlog_filter *)malloc(sizeof(struct enxlog_filter));
    result->entries = NULL;

    enxlog_filter_config_transform_recursively(obj->root->child, &result->entries);

    return result;
}

void enxlog_filter_config_transform_destroy(
     struct enxlog_filter *obj)
{
    enxlog_filter_config_transform_destroy_recursively(obj->entries);
    free(obj);
}

static void enxlog_filter_config_transform_recursively(
    struct enxlog_filter_config_entry *config_entries,
    struct enxlog_filter_entry **filter_entries)
{

    struct enxlog_filter_config_entry *source = config_entries;
    size_t count = 0;
    while (source) {
        count++;
        source = source->next;
    }

    *filter_entries = malloc(sizeof(struct enxlog_filter_entry) * (count + 1));

    struct enxlog_filter_entry *dest = *filter_entries;
    source = config_entries;

    while (source) {

        dest->path = strdup(source->path);
        dest->loglevel = source->loglevel;

        enxlog_filter_config_transform_recursively(source->child, &dest->children);

        source = source->next;
        dest++;
    }

    // Terminate
    dest->children = NULL;
    dest->path = NULL;
    dest->loglevel = LOGLEVEL_NONE;
}

static void enxlog_filter_config_transform_destroy_recursively(
    struct enxlog_filter_entry *filter_entries)
{
    struct enxlog_filter_entry *ptr = filter_entries;
    while (ptr->path) {
        free(ptr->path);
        enxlog_filter_config_transform_destroy_recursively(ptr->children);
        ptr++;
    }

    free(filter_entries);
}
