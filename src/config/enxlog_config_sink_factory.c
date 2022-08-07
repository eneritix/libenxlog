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

#include <enx/log/config/enxlog_config_sink_factory.h>
#include <enx/log/sinks/enxlog_sink_stdout.h>
#include <enx/log/sinks/enxlog_sink_stdout_color.h>
#include <enx/log/sinks/enxlog_sink_file.h>

#include <string.h>


static bool enxlog_config_sink_factory_create_stdout_sink(
        struct enxlog_sink* sink,
        const struct enxlog_config_sink_parameters* parameters,
        enxlog_config_parser_error_callback_t error_callback)
{
    sink->fn_output = enxlog_sink_stdout;
    sink->fn_shutdown = NULL;
    sink->context = NULL;

    return true;
}

static bool enxlog_config_sink_factory_create_stdout_color_sink(
        struct enxlog_sink* sink,
        const struct enxlog_config_sink_parameters* parameters,
        enxlog_config_parser_error_callback_t error_callback)
{
    sink->fn_output = enxlog_sink_stdout_color;
    sink->fn_shutdown = NULL;
    sink->context = NULL;

    return true;
}

static bool enxlog_config_sink_factory_create_file_sink(
        struct enxlog_sink* sink,
        const struct enxlog_config_sink_parameters* parameters,
        enxlog_config_parser_error_callback_t error_callback)
{
    const char* path = enxlog_config_sink_parameters_find(parameters, "path");
    if (path == NULL) {
        error_callback(0, 0, "File sink should specify 'path'");
        return false;
    }

    struct enxlog_sink_file_context* context = enxlog_sink_file_create();
    if (enxlog_sink_file_init(context, path) == -1) {
        error_callback(0, 0, "Could not open log file");
        free(context);
        return false;
    }

    sink->context = context;
    sink->fn_output = enxlog_sink_file;
    sink->fn_shutdown = enxlog_sink_file_destroy;

    return true;
}


bool enxlog_config_sink_factory_create_sink(
    struct enxlog_sink* sink,
    const struct enxlog_config_sink_parameters* parameters,
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback,
    enxlog_config_parser_error_callback_t error_callback)
{

    const char* value = enxlog_config_sink_parameters_find(parameters, "type");
    if (value == NULL) {
        error_callback(0, 0, "Sink type not specified");
        return false;
    }

    if (strcmp(value, "stdout") == 0) {
        return enxlog_config_sink_factory_create_stdout_sink(sink, parameters, error_callback);

    } else if (strcmp(value, "stdout_color") == 0) {
        return enxlog_config_sink_factory_create_stdout_color_sink(sink, parameters, error_callback);

    } else if (strcmp(value, "file") == 0) {
        return enxlog_config_sink_factory_create_file_sink(sink, parameters, error_callback);

    } else {
        if (sink_creation_callback) {
            return sink_creation_callback(sink, parameters);
        } else {
            return false;
        }
    }
}
