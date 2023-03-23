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

#include "test_utils.h"
#include <stdio.h>


static void print_filter_tree_entry(const struct enxlog_filter_entry *entry, int indent)
{
    int i;
    while (entry->name_part) {

        for (i=0; i < indent; ++i) {
            printf("  ");
        }

        printf("%s: %s\n", entry->name_part, loglevel_to_string(entry->loglevel));

        print_filter_tree_entry(entry->children, indent+1);

        ++entry;
    }
}


void print_filter_tree(const struct enxlog_filter *filter)
{
    print_filter_tree_entry(filter->entries, 0);
}

const char *loglevel_to_string(enum enxlog_loglevel loglevel)
{
    switch (loglevel) {
    case LOGLEVEL_NONE: return "LOGLEVEL_NONE";
    case LOGLEVEL_ERROR: return "LOGLEVEL_ERROR";
    case LOGLEVEL_WARN: return "LOGLEVEL_WARN";
    case LOGLEVEL_INFO: return "LOGLEVEL_INFO";
    case LOGLEVEL_DEBUG: return "LOGLEVEL_DEBUG";
    default: return "";
    }
}
