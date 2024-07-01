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

#include <enx/log/sinks/enxlog_sink_stdout_color.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


#define LOG_COLOR_BROWN "\x1b[33m"
#define LOG_COLOR_WHITE "\x1b[97m"
#define LOG_COLOR_RED "\x1b[31m"
#define LOG_COLOR_GREEN "\x1b[32m"
#define LOG_COLOR_DARK_GRAY "\x1b[90m"
#define LOG_COLOR_YELLOW "\x1b[93m"
#define LOG_COLOR_RESET "\x1b[0m"

struct enxlog_sink_stdout_color_context *enxlog_sink_stdout_color_create()
{
    return malloc(sizeof(struct enxlog_sink_stdout_color_context));
}

void enxlog_sink_stdout_color_destroy(void *context)
{
    struct enxlog_sink_stdout_color_context *ctx = (struct enxlog_sink_stdout_color_context *)context;
    free(ctx);
}

bool enxlog_sink_stdout_color_init(void *context)
{
    return true;
}

void enxlog_sink_stdout_color_shutdown(void *context)
{

}

void enxlog_sink_stdout_color_log_entry_open(
    void* context,
    const struct enxlog_logger *logger,
    enum enxlog_loglevel loglevel,
    const char *func,
    unsigned int line)
{

    struct enxlog_sink_stdout_color_context *ctx = (struct enxlog_sink_stdout_color_context *)context;
    ctx->tag_length = 0;

    // Timestamp
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    struct tm* timeinfo;
    timeinfo = localtime(&curTime.tv_sec);

    char tag[64];
    strftime(tag, sizeof(tag), "%Y-%m-%d %H:%M:%S", timeinfo);
    ctx->tag_length += printf(LOG_COLOR_DARK_GRAY "%s.%03d" LOG_COLOR_RESET " ", tag, milli);

    // Severity
    switch (loglevel) {
        case LOGLEVEL_ERROR: ctx->tag_length += printf(LOG_COLOR_RED   "-- ERROR -- " LOG_COLOR_RESET); break;
        case LOGLEVEL_WARN:  ctx->tag_length += printf(LOG_COLOR_YELLOW "-- WARN  -- " LOG_COLOR_RESET); break;
        case LOGLEVEL_INFO:  ctx->tag_length += printf(LOG_COLOR_RESET  "-- INFO  -- "); break;
        case LOGLEVEL_DEBUG: ctx->tag_length += printf(LOG_COLOR_RESET "-- DEBUG -- "); break;
        case LOGLEVEL_TRACE: ctx->tag_length += printf(LOG_COLOR_RESET "-- TRACE -- "); break;
        default : break;
    }


    ctx->tag_length += printf("%s", LOG_COLOR_BROWN);

    // Path
    const char **name_part = logger->name;
    while (*name_part) {
        ctx->tag_length += printf("%s::", *name_part);
        name_part++;
    }

    // Function and line
    ctx->tag_length += printf( "%s:%u" LOG_COLOR_RESET ": ", func, line);
}

void enxlog_sink_stdout_color_log_entry_write(
    void *context,
    const char *ptr,
    size_t length)
{
    struct enxlog_sink_stdout_color_context *ctx = (struct enxlog_sink_stdout_color_context *)context;

    if ((length == 1) && *ptr == '\n') {
        putc('\n', stdout);
        for (size_t i=0; i < ctx->tag_length; ++i) {
            putc(' ', stdout);
        }

    } else {
        fwrite(ptr, 1, length, stdout);
    }
}

void enxlog_sink_stdout_color_log_entry_close(
    void *context)
{
    (void)context;
    printf("\n");
}
