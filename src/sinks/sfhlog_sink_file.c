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

#include <sfhlog/sinks/sfhlog_sink_file.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


struct sfhlog_sink_file_context* sfhlog_sink_file_create()
{
	return malloc(sizeof(struct sfhlog_sink_file_context));
}

void sfhlog_sink_file_destroy(void* context)
{
	struct sfhlog_sink_file_context* ctx = (struct sfhlog_sink_file_context*)context;

	sfhlog_sink_file_deinit(ctx);
	free(ctx);
}


int sfhlog_sink_file_init(struct sfhlog_sink_file_context* ctx, const char* path)
{
	ctx->file = fopen(path, "a");
	return (ctx->file == NULL) ? -1 : 0;
}

void sfhlog_sink_file_deinit(struct sfhlog_sink_file_context* ctx)
{
	fclose(ctx->file);
}

void sfhlog_sink_file(
	void* context,
	const struct sfhlog_logger* logger,
	enum sfhlog_severity severity,
	const char* func,
	unsigned int line,
	const char* fmt,
	va_list ap)
{
	struct sfhlog_sink_file_context* ctx = (struct sfhlog_sink_file_context*)context;

	// Timestamp
	struct timeval curTime;
	gettimeofday(&curTime, NULL);
	int milli = curTime.tv_usec / 1000;

	struct tm* timeinfo;
	timeinfo = localtime(&curTime.tv_sec);

	char tag[64];
	strftime(tag, sizeof(tag), "%Y-%m-%d %H:%M:%S", timeinfo);
	fprintf(ctx->file, "%s.%03d ", tag, milli);

	// Severity
	switch (severity) {
		case LOG_SEVERITY_ERROR: fprintf(ctx->file, "-- ERROR -- "); break;
		case LOG_SEVERITY_WARN: fprintf(ctx->file, "-- WARN  -- "); break;
		case LOG_SEVERITY_INFO: fprintf(ctx->file, "-- INFO  -- "); break;
		case LOG_SEVERITY_DEBUG: fprintf(ctx->file, "-- DEBUG -- "); break;
	}


	// Path
	const char** path = logger->path;
	while (*path) {
		fprintf(ctx->file, "%s::", *path);
		path++;
	}

	// Function and line
	fprintf(ctx->file, "%s:%u: ", func, line);

	// Message
	vfprintf(ctx->file, fmt, ap);
	fprintf(ctx->file, "\n");

	fflush(ctx->file);
}
