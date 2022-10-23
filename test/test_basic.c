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


LOGGER_DECLARE(logger_one, "one");
LOGGER_DECLARE(logger_two, "one", "two");
LOGGER_DECLARE(logger_three, "one", "two", "three");


enxlog_filter(filter_tree)
    enxlog_filter_entry("one", LOGLEVEL_INFO)
        enxlog_filter_entry("two", LOGLEVEL_INFO)
            enxlog_filter_entry("three", LOGLEVEL_ERROR)
            enxlog_end_filter_entry()
        enxlog_end_filter_entry()
    enxlog_end_filter_entry()
enxlog_end_filter()


enxlog_sink_list(sink_list)
    enxlog_sink(0, &enxlog_sink_stdout, 0)
enxlog_end_sink_list()



int main(void)
{

    print_filter_tree(filter_tree);

    enxlog_init(LOGLEVEL_NONE, sink_list, NULL, filter_tree);

    LOG_ERROR(logger_one, "This should print");
    LOG_WARN(logger_two, "This should print");
    LOG_INFO(logger_three, "This should print");

    return 0;
}
