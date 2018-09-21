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

#include <sfhlog/sfhlog.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


static const struct sfhlog_config* _sfhlog_config = NULL;


void sfhlog_init(const struct sfhlog_config* config)
{
	_sfhlog_config = config;
}

void sfhlog_log(
	const struct sfhlog_logger* logger,
	enum sfhlog_severity severity,
	const char* func,
	unsigned int line,
	const char* fmt, ...)
{
	enum sfhlog_severity config_severity = _sfhlog_config->root->severity;
	const struct sfhlog_config_node* config_node = _sfhlog_config->root->children;
	const char** path = logger->path;

	while (*path) {

		while (config_node->path) {
			if (strcmp(config_node->path, *path) == 0) {
				config_severity = config_node->severity;
				config_node = config_node->children;
				path++;
				break;
			} else {
				config_node++;
			}
		}

		if (config_node->path == 0) {
			break;
		}
	}

	if (severity > config_severity) {
		return;
	}

	// Log
	va_list args;
	va_start(args, fmt);

	const struct sfhlog_sink* sink = _sfhlog_config->sinks;
	while (sink->output_function) {
		sink->output_function(sink->context, logger, severity, func, line, fmt, args);
		sink++;
	}

	va_end(args);
}
