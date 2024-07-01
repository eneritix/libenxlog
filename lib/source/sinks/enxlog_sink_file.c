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

#include <enx/log/sinks/enxlog_sink_file.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


struct enxlog_sink_file_context *enxlog_sink_file_create()
{
    return malloc(sizeof(struct enxlog_sink_file_context));
}

void enxlog_sink_file_destroy(void *context)
{
    struct enxlog_sink_file_context *ctx = (struct enxlog_sink_file_context *)context;

    enxlog_sink_file_shutdown(ctx);
    free(ctx);
}


bool enxlog_sink_file_init(void *context)
{
    struct enxlog_sink_file_context *ctx = (struct enxlog_sink_file_context *)context;

    ctx->file = fopen(ctx->path, "a");
    return (ctx->file != NULL);
}

void enxlog_sink_file_shutdown(void *context)
{
    struct enxlog_sink_file_context *ctx = (struct enxlog_sink_file_context *)context;

    fclose(ctx->file);
}

void enxlog_sink_file_log_entry_open(
    void* context,
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel,
    const char *func,
    unsigned int line)
{
    struct enxlog_sink_file_context *ctx = (struct enxlog_sink_file_context *)context;

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
    switch (loglevel) {
        case LOGLEVEL_ERROR: fprintf(ctx->file, "-- ERROR -- "); break;
        case LOGLEVEL_WARN: fprintf(ctx->file, "-- WARN  -- "); break;
        case LOGLEVEL_INFO: fprintf(ctx->file, "-- INFO  -- "); break;
        case LOGLEVEL_DEBUG: fprintf(ctx->file, "-- DEBUG -- "); break;
        case LOGLEVEL_TRACE: fprintf(ctx->file, "-- TRACE -- "); break;
        default : break;
    }


    // Path
    const char **name_part = logger->name;
    while (*name_part) {
        fprintf(ctx->file, "%s::", *name_part);
        name_part++;
    }

    // Function and line
    fprintf(ctx->file, "%s:%u: ", func, line);

}

void enxlog_sink_file_log_entry_write(
    void *context,
    const char *ptr,
    size_t length)
{
    struct enxlog_sink_file_context *ctx = (struct enxlog_sink_file_context *)context;

    fwrite(ptr, 1, length, ctx->file);
}

void enxlog_sink_file_log_entry_close(
    void *context)
{
    struct enxlog_sink_file_context *ctx = (struct enxlog_sink_file_context *)context;

    fprintf(ctx->file, "\n");
    fflush(ctx->file);
}
