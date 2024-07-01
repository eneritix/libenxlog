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
#include <stdlib.h>

#include "test_utils.h"


LOGGER(logger_one, "one");
LOGGER(logger_two, "one", "two");
LOGGER(logger_three, "one", "two", "three");


enxlog_filter(filter_tree)
    enxlog_filter_entry("one", LOGLEVEL_INFO)
        enxlog_filter_entry("two", LOGLEVEL_INFO)
            enxlog_filter_entry("three", LOGLEVEL_ERROR)
            enxlog_end_filter_entry()
        enxlog_end_filter_entry()
    enxlog_end_filter_entry()
enxlog_end_filter()

static struct enxlog_sink_stdout_context sink_stdout_context;

enxlog_sink_list(sink_list)
    enxlog_sink(
        &sink_stdout_context,
        NULL,
        NULL,
        enxlog_sink_stdout_log_entry_open,
        enxlog_sink_stdout_log_entry_write,
        enxlog_sink_stdout_log_entry_close
    )
enxlog_end_sink_list()



int main(void)
{
    size_t i;

    print_filter_tree(filter_tree);

    enxlog_init(LOGLEVEL_NONE, sink_list, NULL, filter_tree);

    LOG_ERROR(logger_one, "This should print");
    LOG_WARN(logger_two, "This should print");
    LOG_ERROR(logger_three, "This should print");
    LOG_ERROR(logger_three, "Test\nnewline\nalignment");

    unsigned int value = 1234;
    LOG_INFO(
        logger_one,
        "f_int={}, f_uint={}, f_h8={}, f_h16={}, f_h32={}",
        f_int(value),
        f_uint(value),
        f_h8(value),
        f_h16(value),
        f_h32(value));

    unsigned char hexbuf[] = { 0xA5, 0xB2, 0xAA, 0xFF };
    LOG_INFO(logger_one, "hexbuf=[ {} ]", f_h8_array(hexbuf, sizeof(hexbuf)));

    unsigned char largebuf[256];
    for (i=0; i < sizeof(largebuf); ++i) {
        largebuf[i] = i;
    }
    LOG_INFO(logger_one, "largebuf=[ {} ]", f_h8_array(largebuf, sizeof(largebuf)));

    return 0;
}
