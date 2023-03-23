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


LOGGER(logger, "logger");


enxlog_filter(filter_tree)
    enxlog_filter_entry("logger", LOGLEVEL_INFO)
    enxlog_end_filter_entry()
enxlog_end_filter()


enxlog_sink_list(sink_list)
    enxlog_sink(
        NULL,
        NULL,
        NULL,
        enxlog_sink_stdout_log_entry_open,
        enxlog_sink_stdout_log_entry_write,
        enxlog_sink_stdout_log_entry_close
    )
enxlog_end_sink_list()



struct some_struct
{
    uint8_t tag;
    uint32_t length;
    const char *value;
};

static void enxtxt_fstr_fmt_some_struct(const struct enxtxt_fstr_arg *, enxtxt_fstr_output_function_t, void *);

#define f_some_struct(_ptr) \
    { .fn_fmt = enxtxt_fstr_fmt_some_struct, .value._user = _ptr }



int main(void)
{
    size_t i;

    print_filter_tree(filter_tree);

    enxlog_init(LOGLEVEL_NONE, sink_list, NULL, filter_tree);

    struct some_struct some_struct = {
        .tag = 0xA5,
        .length = 5,
        .value = "Hello"
    };

    LOG_INFO(
        logger,
        "f_some_struct={}",
        f_some_struct(&some_struct));


    return 0;
}

static void enxtxt_fstr_fmt_some_struct(
    const struct enxtxt_fstr_arg *arg,
    enxtxt_fstr_output_function_t output_fn,
    void *output_fn_context)
{
    const struct some_struct *some_struct = (const struct some_struct *)arg->value._user;
    enxtxt_fstr_cb(
        output_fn,
        output_fn_context,
        "[ some_struct: tag={}, length={}, value={} ]",
        f_h8(some_struct->tag),
        f_uint(some_struct->length),
        f_str(some_struct->value)
        );
}
