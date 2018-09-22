/*
	Copyright (c) 2018 Eneritix (Pty) Ltd

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

#ifndef SFHLOG_CONFIG_SINK_PARAMETERS_H
#define SFHLOG_CONFIG_SINK_PARAMETERS_H

#include <stdlib.h>
#include <sys/cdefs.h>

__BEGIN_DECLS


struct sfhlog_config_sink_parameter
{
	char* name;
	char* value;
};

struct sfhlog_config_sink_parameters
{
	size_t size;
	size_t count;
	size_t grow;

	struct sfhlog_config_sink_parameter* parameters;
};

struct sfhlog_config_sink_parameters* sfhlog_config_sink_parameters_create();
void sfhlog_config_sink_parameters_destroy(struct sfhlog_config_sink_parameters* parameters);
void sfhlog_config_sink_parameters_add(struct sfhlog_config_sink_parameters* parameters, const char* name, const char* value);
const char* sfhlog_config_sink_parameters_find(const struct sfhlog_config_sink_parameters* parameters, const char* name);


struct sfhlog_config_sink_parameter_list
{
	struct sfhlog_config_sink_parameters** list;
	size_t size;
	size_t count;
	size_t grow;
};

void sfhlog_config_sink_parameter_list_init(struct sfhlog_config_sink_parameter_list* list);
void sfhlog_config_sink_parameter_list_destroy(struct sfhlog_config_sink_parameter_list* list);
void sfhlog_config_sink_parameter_list_add(struct sfhlog_config_sink_parameter_list* list, struct sfhlog_config_sink_parameters* parameters);
size_t sfhlog_config_sink_parameter_list_get_count(struct sfhlog_config_sink_parameter_list* list);
struct sfhlog_config_sink_parameters* sfhlog_config_sink_parameter_list_get_at(struct sfhlog_config_sink_parameter_list* list, size_t index);


__END_DECLS

#endif
