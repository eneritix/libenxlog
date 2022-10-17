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

#include "enxlog_config_sink_list.h"

#include <stdlib.h>
#include <string.h>

struct enxlog_config_sink_list *enxlog_config_sink_list_create()
{
    struct enxlog_config_sink_list *result = malloc(sizeof(struct enxlog_config_sink_list));

    result->sink_count = 0;
    result->sinks = NULL;

    return result;
}

void enxlog_config_sink_list_destroy(struct enxlog_config_sink_list *sink_list)
{
    size_t i = 0;

    for (i=0; i < sink_list->sink_count; ++i) {
        if (sink_list->sinks[i].fn_shutdown) {
            sink_list->sinks[i].fn_shutdown(sink_list->sinks[i].context);
        }
    }

    free(sink_list->sinks);
    free(sink_list);
}

struct enxlog_sink *enxlog_config_sink_list_append(struct enxlog_config_sink_list *sink_list)
{
    static struct enxlog_sink * result = NULL;

    sink_list->sink_count++;
    sink_list->sinks = realloc(sink_list->sinks, sizeof(struct enxlog_sink) * sink_list->sink_count);

    result = &sink_list->sinks[sink_list->sink_count - 1];

    result->context = NULL;
    result->fn_output = NULL;
    result->fn_shutdown = NULL;

    return result;
}
