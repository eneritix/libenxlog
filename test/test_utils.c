#include "test_utils.h"
#include <stdio.h>


static void print_filter_tree_entry(const struct enxlog_filter_entry *entry, int indent)
{
    int i;
    while (entry->path) {

        for (i=0; i < indent; ++i) {
            printf("  ");
        }

        printf("%s: %s\n", entry->path, loglevel_to_string(entry->loglevel));

        print_filter_tree_entry(entry->children, indent+1);

        ++entry;
    }
}


void print_filter_tree(const struct enxlog_filter_entry *filter_list)
{
    print_filter_tree_entry(filter_list, 0);
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
