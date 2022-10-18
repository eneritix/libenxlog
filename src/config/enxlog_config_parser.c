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

#include <enx/log/config/enxlog_config_parser.h>
#include <enx/log/config/enxlog_config_sink_parameters.h>
#include <enx/log/config/enxlog_config_sink_factory.h>

#include "enxlog_config_filter_tree.h"
#include "enxlog_config_sink_list.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <yaml.h>


struct enxlog_config
{
    struct enxlog_config_sink_list *sink_list;
    struct enxlog_filter_entry *filter_tree;
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback;
    enxlog_config_parser_error_callback_t error_callback;
    enum enxlog_loglevel default_loglevel;
};


static bool enxlog_config_parse_root(struct enxlog_config *config, yaml_parser_t *parser);
static bool enxlog_config_parse_sections(struct enxlog_config *config, yaml_parser_t *parser);
static bool enxlog_config_parse_section_options(struct enxlog_config *config, yaml_parser_t *parser);
static bool enxlog_config_parse_section_sink(struct enxlog_config *config, yaml_parser_t *parser);
static bool enxlog_config_parse_section_filter(struct enxlog_config *config, yaml_parser_t *parser);
static enum enxlog_loglevel enxlog_config_parse_loglevel(const char* name);
static void enxlog_config_parse_configuration_option(struct enxlog_config *config, const char *key, const char *value);



struct enxlog_config *enxlog_config_parse(
    const char* path,
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback,
    enxlog_config_parser_error_callback_t error_callback)
{
    struct enxlog_config *config = (struct enxlog_config *)malloc(sizeof(struct enxlog_config));
    yaml_parser_t parser;

    config->sink_list = enxlog_config_sink_list_create();
    config->filter_tree = enxlog_config_filter_tree_create();
    config->sink_creation_callback = sink_creation_callback;
    config->error_callback = error_callback;
    config->default_loglevel = LOGLEVEL_NONE;

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        goto error_fopen;
    }

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    if (!enxlog_config_parse_root(config, &parser)) {
        goto error_enxlog_config_parse_root;
    }

    // Terminate the sink list
    enxlog_config_sink_list_append(config->sink_list);

    // Close the file
    fclose(file);

    // Destroy the parser
    yaml_parser_delete(&parser);

    return config;

error_enxlog_config_parse_root:

    yaml_parser_delete(&parser);
    fclose(file);

error_fopen:
    enxlog_config_destroy(config);

    return NULL;
}

void enxlog_config_destroy(struct enxlog_config* config)
{
    enxlog_config_filter_tree_destroy(config->filter_tree);
    enxlog_config_sink_list_destroy(config->sink_list);
    free(config);
}

enum enxlog_loglevel enxlog_config_get_default_loglevel(struct enxlog_config* config)
{
    return config->default_loglevel;
}

const struct enxlog_sink *enxlog_config_get_sink_list(struct enxlog_config* config)
{
    return config->sink_list->sinks;
}

const struct enxlog_filter_entry *enxlog_config_get_filter_tree(struct enxlog_config* config)
{
    return config->filter_tree->children;
}

static bool enxlog_config_parse_root(struct enxlog_config *config, yaml_parser_t *parser)
{
    yaml_event_t event;
    enum {
        PARSE_STATE_START_STREAM,
        PARSE_STATE_START_DOCUMENT,
        PARSE_STATE_START_MAPPING,
        PARSE_STATE_SUCCESS,
        PARSE_STATE_ERROR
    } parse_state;

    parse_state = PARSE_STATE_START_STREAM;

    while ((parse_state != PARSE_STATE_SUCCESS) && (parse_state != PARSE_STATE_ERROR)) {
        if (yaml_parser_parse(parser, &event)) {

            switch (parse_state) {
                case PARSE_STATE_START_STREAM: {
                    switch (event.type) {
                        case YAML_STREAM_START_EVENT: {
                            parse_state = PARSE_STATE_START_DOCUMENT;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                case PARSE_STATE_START_DOCUMENT: {
                    switch (event.type) {
                        case YAML_DOCUMENT_START_EVENT: {
                            parse_state = PARSE_STATE_START_MAPPING;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                case PARSE_STATE_START_MAPPING: {
                    switch (event.type) {
                        case YAML_MAPPING_START_EVENT: {
                            parse_state = PARSE_STATE_SUCCESS;
                            if (!enxlog_config_parse_sections(config, parser)) {
                                parse_state = PARSE_STATE_ERROR;
                            }
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                default: break;
            }

            yaml_event_delete(&event);

        } else {
            parse_state = PARSE_STATE_ERROR;
            config->error_callback(
                parser->problem_mark.line,
                parser->problem_mark.column,
                parser->problem);
        }
    }

    return (parse_state == PARSE_STATE_SUCCESS);
}

static bool enxlog_config_parse_sections(struct enxlog_config *config, yaml_parser_t *parser)
{
    yaml_event_t event;
    char *label = NULL;

    enum {
        PARSE_STATE_SECTION_LABEL,
        PARSE_STATE_SECTION_CONTENT,
        PARSE_STATE_SUCCESS,
        PARSE_STATE_ERROR
    } parse_state;

    parse_state = PARSE_STATE_SECTION_LABEL;

    while ((parse_state != PARSE_STATE_SUCCESS) && (parse_state != PARSE_STATE_ERROR)) {
        if (yaml_parser_parse(parser, &event)) {

            switch (parse_state) {
                case PARSE_STATE_SECTION_LABEL: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(label);
                            label = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            parse_state = PARSE_STATE_SECTION_CONTENT;
                        } break;
                        case YAML_MAPPING_END_EVENT: {
                            parse_state = PARSE_STATE_SUCCESS;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                case PARSE_STATE_SECTION_CONTENT: {
                    switch (event.type) {
                        case YAML_MAPPING_START_EVENT: {
                            parse_state = PARSE_STATE_SECTION_LABEL;

                            if (strcmp(label, "options") == 0) {
                                if (!enxlog_config_parse_section_options(config, parser)) {
                                    parse_state = PARSE_STATE_ERROR;
                                }
                            }

                            else if (strcmp(label, "sink") == 0) {
                                if (!enxlog_config_parse_section_sink(config, parser)) {
                                    parse_state = PARSE_STATE_ERROR;
                                }
                            }

                            else if (strcmp(label, "filter") == 0) {
                                if (!enxlog_config_parse_section_filter(config, parser)) {
                                    parse_state = PARSE_STATE_ERROR;
                                }
                            }

                            else {
                                parse_state = PARSE_STATE_ERROR;
                                config->error_callback(
                                    event.start_mark.line,
                                    event.start_mark.column,
                                    "Unexpected input");
                            }

                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                default: break;
            }

            yaml_event_delete(&event);

        } else {
            parse_state = PARSE_STATE_ERROR;
            config->error_callback(
                parser->problem_mark.line,
                parser->problem_mark.column,
                parser->problem);
        }
    }

    free(label);

    return (parse_state == PARSE_STATE_SUCCESS);
}

static bool enxlog_config_parse_section_options(struct enxlog_config *config, yaml_parser_t *parser)
{
    yaml_event_t event;
    char *key = NULL;
    char *value = NULL;

    enum {
        PARSE_STATE_KEY,
        PARSE_STATE_VALUE,
        PARSE_STATE_SUCCESS,
        PARSE_STATE_ERROR
    } parse_state;

    parse_state = PARSE_STATE_KEY;

    while ((parse_state != PARSE_STATE_SUCCESS) && (parse_state != PARSE_STATE_ERROR)) {
        if (yaml_parser_parse(parser, &event)) {

            switch (parse_state) {
                case PARSE_STATE_KEY: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(key);
                            key = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            parse_state = PARSE_STATE_VALUE;
                        } break;
                        case YAML_MAPPING_END_EVENT: {
                            parse_state = PARSE_STATE_SUCCESS;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                case PARSE_STATE_VALUE: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(value);
                            value = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            enxlog_config_parse_configuration_option(config, key, value);
                            parse_state = PARSE_STATE_KEY;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                default: break;
            }

            yaml_event_delete(&event);

        } else {
            parse_state = PARSE_STATE_ERROR;
            config->error_callback(
                parser->problem_mark.line,
                parser->problem_mark.column,
                parser->problem);
        }
    }

    free(key);
    free(value);

    return (parse_state == PARSE_STATE_SUCCESS);
}

static bool enxlog_config_parse_section_sink(struct enxlog_config *config, yaml_parser_t *parser)
{
    yaml_event_t event;
    char *key = NULL;
    char *value = NULL;

    enum {
        PARSE_STATE_KEY,
        PARSE_STATE_VALUE,
        PARSE_STATE_SUCCESS,
        PARSE_STATE_ERROR
    } parse_state;

    parse_state = PARSE_STATE_KEY;

    struct enxlog_config_sink_parameters *sink_parameters = enxlog_config_sink_parameters_create();

    while ((parse_state != PARSE_STATE_SUCCESS) && (parse_state != PARSE_STATE_ERROR)) {
        if (yaml_parser_parse(parser, &event)) {

            switch (parse_state) {
                case PARSE_STATE_KEY: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(key);
                            key = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            parse_state = PARSE_STATE_VALUE;
                        } break;
                        case YAML_MAPPING_END_EVENT: {
                            parse_state = PARSE_STATE_SUCCESS;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                case PARSE_STATE_VALUE: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(value);
                            value = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            enxlog_config_sink_parameters_add(sink_parameters, key, value);
                            parse_state = PARSE_STATE_KEY;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                default: break;
            }

            yaml_event_delete(&event);

        } else {
            parse_state = PARSE_STATE_ERROR;
            config->error_callback(
                parser->problem_mark.line,
                parser->problem_mark.column,
                parser->problem);
        }
    }

    if (parse_state == PARSE_STATE_SUCCESS) {
        struct enxlog_sink *sink = enxlog_config_sink_list_append(config->sink_list);
        enxlog_config_sink_factory_create_sink(
            sink,
            sink_parameters,
            config->sink_creation_callback,
            config->error_callback);
    }

    enxlog_config_sink_parameters_destroy(sink_parameters);
    free(key);
    free(value);

    return (parse_state == PARSE_STATE_SUCCESS);
}

static bool enxlog_config_parse_section_filter(struct enxlog_config *config, yaml_parser_t *parser)
{
    yaml_event_t event;
    char *key = NULL;
    char *value = NULL;

    enum {
        PARSE_STATE_KEY,
        PARSE_STATE_VALUE,
        PARSE_STATE_SUCCESS,
        PARSE_STATE_ERROR
    } parse_state;

    parse_state = PARSE_STATE_KEY;

    while ((parse_state != PARSE_STATE_SUCCESS) && (parse_state != PARSE_STATE_ERROR)) {
        if (yaml_parser_parse(parser, &event)) {

            switch (parse_state) {
                case PARSE_STATE_KEY: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(key);
                            key = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            parse_state = PARSE_STATE_VALUE;
                        } break;
                        case YAML_MAPPING_END_EVENT: {
                            parse_state = PARSE_STATE_SUCCESS;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                case PARSE_STATE_VALUE: {
                    switch (event.type) {
                        case YAML_SCALAR_EVENT: {
                            free(value);
                            value = strndup((const char *)event.data.scalar.value, event.data.scalar.length);
                            enxlog_config_filter_tree_append(config->filter_tree, key, enxlog_config_parse_loglevel(value));
                            parse_state = PARSE_STATE_KEY;
                        } break;
                        default: {
                            parse_state = PARSE_STATE_ERROR;
                            config->error_callback(
                                event.start_mark.line,
                                event.start_mark.column,
                                "Unexpected input");
                        } break;
                    }
                } break;
                default: break;
            }

            yaml_event_delete(&event);

        } else {
            parse_state = PARSE_STATE_ERROR;
            config->error_callback(
                parser->problem_mark.line,
                parser->problem_mark.column,
                parser->problem);
        }
    }

    free(key);
    free(value);

    return (parse_state == PARSE_STATE_SUCCESS);
}

static enum enxlog_loglevel enxlog_config_parse_loglevel(const char* name)
{
    if (strcmp(name, "NONE") == 0) {
        return LOGLEVEL_NONE;

    } else if (strcmp(name, "ERROR") == 0) {
        return LOGLEVEL_ERROR;

    } else if (strcmp(name, "WARN") == 0) {
        return LOGLEVEL_WARN;

    } else if (strcmp(name, "INFO") == 0) {
        return LOGLEVEL_INFO;

    } else if (strcmp(name, "DEBUG") == 0) {
        return LOGLEVEL_DEBUG;
    }

    return LOGLEVEL_NONE;
}

static void enxlog_config_parse_configuration_option(struct enxlog_config *config, const char *key, const char *value)
{
    if (strcmp(key, "default_loglevel") == 0) {
        config->default_loglevel = enxlog_config_parse_loglevel(value);
    }
}
