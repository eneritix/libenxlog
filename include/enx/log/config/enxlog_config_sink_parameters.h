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

#ifndef ENXLOG_CONFIG_SINK_PARAMETERS_H
#define ENXLOG_CONFIG_SINK_PARAMETERS_H

#include <stdlib.h>
#include <sys/cdefs.h>

__BEGIN_DECLS


struct enxlog_config_sink_parameter
{
    char* name;
    char* value;
};

struct enxlog_config_sink_parameters
{
    size_t size;
    size_t count;
    size_t grow;

    struct enxlog_config_sink_parameter* parameters;
};

struct enxlog_config_sink_parameters* enxlog_config_sink_parameters_create();
void enxlog_config_sink_parameters_destroy(struct enxlog_config_sink_parameters* parameters);
void enxlog_config_sink_parameters_add(struct enxlog_config_sink_parameters* parameters, const char* name, const char* value);
const char* enxlog_config_sink_parameters_find(const struct enxlog_config_sink_parameters* parameters, const char* name);


struct enxlog_config_sink_parameter_list
{
    struct enxlog_config_sink_parameters** list;
    size_t size;
    size_t count;
    size_t grow;
};

void enxlog_config_sink_parameter_list_init(struct enxlog_config_sink_parameter_list* list);
void enxlog_config_sink_parameter_list_destroy(struct enxlog_config_sink_parameter_list* list);
void enxlog_config_sink_parameter_list_add(struct enxlog_config_sink_parameter_list* list, struct enxlog_config_sink_parameters* parameters);
size_t enxlog_config_sink_parameter_list_get_count(struct enxlog_config_sink_parameter_list* list);
struct enxlog_config_sink_parameters* enxlog_config_sink_parameter_list_get_at(struct enxlog_config_sink_parameter_list* list, size_t index);


__END_DECLS

#endif
