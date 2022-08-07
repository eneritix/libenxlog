#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <enx/log/enxlog.h>

void print_filter_tree(const struct enxlog_filter_entry *filter_list);
const char *loglevel_to_string(enum enxlog_loglevel loglevel);

#endif
