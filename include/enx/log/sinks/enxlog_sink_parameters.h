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

#ifndef ENXLOG_SINK_PARAMETERS_H
#define ENXLOG_SINK_PARAMETERS_H

#include <stdlib.h>
#include <sys/cdefs.h>

__BEGIN_DECLS


struct enxlog_sink_parameter
{
    const char* key;
    const char* value;
    struct enxlog_sink_parameter *next;
};

struct enxlog_sink_parameters
{
    struct enxlog_sink_parameter *head;
    struct enxlog_sink_parameter *tail;
};

/**
 * @brief Creates a sink parameters object
 *
 */
struct enxlog_sink_parameters* enxlog_sink_parameters_create();

/**
 * @brief Destroys a sink parameters object
 *
 */
void enxlog_sink_parameters_destroy(struct enxlog_sink_parameters* parameters);

/**
 * @brief Adds a parameter to the parameters object
 *
 */
void enxlog_sink_parameters_add(struct enxlog_sink_parameters* parameters, const char* key, const char* value);

/**
 * @brief Finds a parameter in the parameters object
 * @returns NULL if the key is not found
 *
 */
const char* enxlog_sink_parameters_find(const struct enxlog_sink_parameters* parameters, const char* key);


__END_DECLS

#endif
