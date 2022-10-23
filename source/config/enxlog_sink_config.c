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

#include "enxlog_sink_config.h"
#include <enx/log/enxlog.h>

#include <stdlib.h>
#include <string.h>

/**
 * @brief Creates a configuration entry
 */
static struct enxlog_sink_config_entry* enxlog_sink_config_entry_create(
    struct enxlog_sink_parameters *parameters);

/**
 * @brief Destroys a configuration entry
 */
static void enxlog_sink_config_entry_destroy(struct enxlog_sink_config_entry *obj);



struct enxlog_sink_config *enxlog_sink_config_create()
{
    struct enxlog_sink_config *obj = malloc(sizeof(struct enxlog_sink_config));

    obj->sinks = NULL;

    return obj;

}

void enxlog_sink_config_destroy(struct enxlog_sink_config *obj)
{
    struct enxlog_sink_config_entry *ptr = obj->sinks;
    while (ptr) {
        struct enxlog_sink_config_entry *to_destroy = ptr;
        ptr = ptr->next;
        enxlog_sink_config_entry_destroy(to_destroy);
    }

    free(obj);
}

void enxlog_sink_config_append(
    struct enxlog_sink_config *obj,
    struct enxlog_sink_parameters *parameters)
{
    if (obj->sinks) {
        struct enxlog_sink_config_entry *ptr = obj->sinks;
        while (ptr->next) {
            ptr = ptr->next;
        }

        ptr->next = enxlog_sink_config_entry_create(parameters);

    } else {
        obj->sinks = enxlog_sink_config_entry_create(parameters);
    }
}

struct enxlog_sink *enxlog_sink_config_transform(
    const struct enxlog_sink_config *obj,
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback,
    enxlog_config_parser_error_callback_t error_callback)
{
    struct enxlog_sink *result = NULL;

    struct enxlog_sink_config_entry *source = obj->sinks;
    size_t count = 0;
    size_t initialized = 0;
    size_t i = 0;

    while (source) {
        count++;
        source = source->next;
    }

    result = (struct enxlog_sink *)malloc(sizeof(struct enxlog_sink) * (count + 1));
    source = obj->sinks;

    for (i=0; i < count; ++i) {
        if (sink_creation_callback(&result[i], source->parameters, error_callback)) {
            initialized++;
        } else {
            break;
        }
        source = source->next;
    }

    if (initialized != count) {
        for (i=0; i < initialized; ++i) {
            if (result[i].fn_shutdown) {
                result[i].fn_shutdown(result[i].context);
            }
        }

        free(result);
        result = NULL;

    } else {
        // Terminate
        result[count].context = NULL;
        result[count].fn_output = NULL;
        result[count].fn_shutdown = NULL;
    }

    return result;
}

void enxlog_sink_config_transform_destroy(
     struct enxlog_sink *obj)
{
    struct enxlog_sink *ptr = obj;
    while (ptr->fn_output) {
        if (ptr->fn_shutdown) {
            ptr->fn_shutdown(ptr->context);
        }
        ptr++;
    }

    free(obj);
}

static struct enxlog_sink_config_entry* enxlog_sink_config_entry_create(
    struct enxlog_sink_parameters *parameters)
{
    struct enxlog_sink_config_entry *obj = (struct enxlog_sink_config_entry *)malloc(sizeof(struct enxlog_sink_config_entry));
    obj->parameters = parameters;
    obj->next = NULL;
    return obj;
}

static void enxlog_sink_config_entry_destroy(struct enxlog_sink_config_entry *obj)
{
    if (obj->parameters) {
        enxlog_sink_parameters_destroy(obj->parameters);
    }

    free(obj);
}
