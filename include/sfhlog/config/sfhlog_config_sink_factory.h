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

#ifndef SFHLOG_CONFIG_SINK_FACTORY_H
#define SFHLOG_CONFIG_SINK_FACTORY_H

#include <sfhlog/sfhlog.h>
#include <sfhlog/config/sfhlog_config_parser.h>
#include <sfhlog/config/sfhlog_config_sink_parameters.h>

#include <stdbool.h>
#include <sys/cdefs.h>

__BEGIN_DECLS


bool sfhlog_config_sink_factory_create_sink(
	struct sfhlog_sink* sink,
	const struct sfhlog_config_sink_parameters* parameters,
	sfhlog_config_parser_sink_creation_callback_t sink_creation_callback,
	sfhlog_config_parser_error_callback_t error_callback);


__END_DECLS

#endif
