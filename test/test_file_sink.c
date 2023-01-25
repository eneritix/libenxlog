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

#include <enx/log/enxlog.h>
#include <enx/log/sinks/enxlog_sink_stdout.h>
#include <enx/log/sinks/enxlog_sink_stdout_color.h>
#include <enx/log/sinks/enxlog_sink_file.h>


static struct enxlog_sink_file_context sink_file_context;


enxlog_filter(filter_tree)
enxlog_end_filter()


enxlog_sink_list(sink_list)
    enxlog_sink(
        &sink_file_context,
        enxlog_sink_file_init,
        enxlog_sink_file_shutdown,
        enxlog_sink_file_log_entry_open,
        enxlog_sink_file_log_entry_write,
        enxlog_sink_file_log_entry_close
    )
enxlog_end_sink_list()



LOGGER_DECLARE(logger, "test");


int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("usage: test_file_sink <output_file>\n");
        return 1;
    }

    sink_file_context.path = argv[1];

    if (!enxlog_init(LOGLEVEL_DEBUG, sink_list, NULL, filter_tree)) {
        printf("Could not open output file\n");
        return 1;
    }

    LOG_ERROR(logger, "This is an error");
    LOG_WARN(logger, "This is a warning");
    LOG_INFO(logger, "This is info");
    LOG_DEBUG(logger, "This is debug data");

    return 0;
}
