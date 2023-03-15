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

#include <enx/log/config/enxlog_sink_parameters.h>

#include <stdlib.h>
#include <string.h>

struct enxlog_sink_parameters* enxlog_sink_parameters_create()
{
    struct enxlog_sink_parameters* parameters = malloc(sizeof(struct enxlog_sink_parameters));
    parameters->head = NULL;
    parameters->tail = NULL;

    return parameters;
}

void enxlog_sink_parameters_destroy(struct enxlog_sink_parameters* parameters)
{
    struct enxlog_sink_parameter *parameter = parameters->head;
    while (parameter) {
        struct enxlog_sink_parameter *to_free = parameter;
        parameter = parameter->next;

        free((void *)to_free->key);
        free((void *)to_free->value);
        free(to_free);
    }

    free(parameters);
}

void enxlog_sink_parameters_add(struct enxlog_sink_parameters* parameters, const char* key, const char* value)
{
    struct enxlog_sink_parameter *parameter = malloc(sizeof(struct enxlog_sink_parameter));
    parameter->key = strdup(key);
    parameter->value = strdup(value);
    parameter->next = NULL;

    if (parameters->tail == NULL) {
        parameters->head = parameter;
        parameters->tail = parameter;
    } else {
        parameters->tail->next = parameter;
        parameters->tail = parameter;
    }
}

const char *enxlog_sink_parameters_find(const struct enxlog_sink_parameters* parameters, const char* key)
{
    struct enxlog_sink_parameter *parameter = parameters->head;
    const char *result = NULL;

    while (parameter && !result) {
        if (strcmp(parameter->key, key) == 0) {
            result = parameter->value;
        }

        parameter = parameter->next;
    }

    return result;
}
