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

#include <enx/log/config/enxlog_config_sink_parameters.h>

#include <stdlib.h>
#include <string.h>

struct enxlog_config_sink_parameters* enxlog_config_sink_parameters_create()
{
    struct enxlog_config_sink_parameters* parameters = malloc(sizeof(struct enxlog_config_sink_parameters));

    parameters->size = 4;
    parameters->count = 0;
    parameters->grow = 4;

    parameters->parameters = malloc(sizeof(struct enxlog_config_sink_parameter) * parameters->size);

    return parameters;
}

void enxlog_config_sink_parameters_destroy(struct enxlog_config_sink_parameters* parameters)
{
    for (size_t i=0; i < parameters->count; ++i) {
        free(parameters->parameters[i].name);
        free(parameters->parameters[i].value);
    }

    free(parameters->parameters);
    free(parameters);
}

void enxlog_config_sink_parameters_add(struct enxlog_config_sink_parameters* parameters, const char* name, const char* value)
{
    if (parameters->count == parameters->size) {
        parameters->size += parameters->grow;
        parameters->parameters = realloc(parameters->parameters, sizeof(struct enxlog_config_sink_parameter) * parameters->size);
    }

    parameters->parameters[parameters->count].name = strdup(name);
    parameters->parameters[parameters->count].value = strdup(value);

    parameters->count++;
}

const char* enxlog_config_sink_parameters_find(const struct enxlog_config_sink_parameters* parameters, const char* name)
{
    for (size_t i=0; i < parameters->count; ++i) {
        if (strcmp(parameters->parameters[i].name, name) == 0) {
            return parameters->parameters[i].value;
        }
    }

    return NULL;
}

void enxlog_config_sink_parameter_list_init(struct enxlog_config_sink_parameter_list* list)
{
    list->size = 4;
    list->count = 0;
    list->grow = 4;
    list->list = malloc(sizeof(struct shflog_config_sink_parameters*) * list->size);
}

void enxlog_config_sink_parameter_list_destroy(struct enxlog_config_sink_parameter_list* list)
{
    for (size_t i=0; i < list->count; ++i) {
        enxlog_config_sink_parameters_destroy(list->list[i]);
    }

    free(list->list);
}

void enxlog_config_sink_parameter_list_add(struct enxlog_config_sink_parameter_list* list, struct enxlog_config_sink_parameters* parameters)
{
    if (list->count == list->size) {
        list->size = list->size + list->grow;
        list->list = realloc(list->list, sizeof(struct shflog_config_sink_parameters*) * list->size);
    }

    list->list[list->count++] = parameters;
}

size_t enxlog_config_sink_parameter_list_get_count(struct enxlog_config_sink_parameter_list* list)
{
    return list->count;
}

struct enxlog_config_sink_parameters* enxlog_config_sink_parameter_list_get_at(struct enxlog_config_sink_parameter_list* list, size_t index)
{
    return list->list[index];
}
