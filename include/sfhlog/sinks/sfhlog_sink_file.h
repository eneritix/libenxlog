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

#ifndef SFHLOG_SINK_FILE_H
#define SFHLOG_SINK_FILE_H

#include <sfhlog/sfhlog.h>
#include <stdio.h>


struct sfhlog_sink_file_context
{
	FILE* file;
};


struct sfhlog_sink_file_context* sfhlog_sink_file_create();
void sfhlog_sink_file_destroy(void* context);

int sfhlog_sink_file_init(struct sfhlog_sink_file_context* ctx, const char* path);
void sfhlog_sink_file_deinit(struct sfhlog_sink_file_context* ctx);

void sfhlog_sink_file(
		void* context,
		const struct sfhlog_logger* logger,
		enum sfhlog_severity severity,
		const char* func,
		unsigned int line,
		const char* fmt,
		va_list ap);

#endif
