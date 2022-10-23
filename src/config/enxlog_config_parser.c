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
#include <enx/log/sinks/enxlog_sink_parameters.h>

#include "enxlog_filter_config.h"
#include "enxlog_sink_config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <yaml.h>


struct enxlog_config
{
    struct enxlog_filter_config *filter_config;
    struct enxlog_filter *filter;

    struct enxlog_sink_config *sink_config;
    struct enxlog_sink *sinks;

    enxlog_config_parser_sink_creation_callback_t sink_creation_callback;
    enxlog_config_parser_error_callback_t error_callback;
    enum enxlog_loglevel default_loglevel;
};

typedef void (*enxlog_config_parse_mapping_callback_t)(void *ctx, const char *key, const char *value);


static bool enxlog_config_parse_root(struct enxlog_config *config, yaml_parser_t *parser);
static bool enxlog_config_parse_sections(struct enxlog_config *config, yaml_parser_t *parser);
static void enxlog_config_parse_section_options(void *ctx, const char *key, const char *value);
static void enxlog_config_parse_section_sink(void *ctx, const char *key, const char *value);
static void enxlog_config_parse_section_filter(void *ctx, const char *key, const char *value);
static bool enxlog_config_parse_generic_mapping(
    struct enxlog_config *config,
    yaml_parser_t *parser,
    enxlog_config_parse_mapping_callback_t callback,
    void *context);
static enum enxlog_loglevel enxlog_config_parse_loglevel(const char* name);



struct enxlog_config *enxlog_config_parse(
    const char* path,
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback,
    enxlog_config_parser_error_callback_t error_callback)
{
    struct enxlog_config *config = (struct enxlog_config *)malloc(sizeof(struct enxlog_config));
    yaml_parser_t parser;

    // Create the filter config
    config->filter_config = enxlog_filter_config_create();
    config->filter = NULL;

    // Create the sink config
    config->sink_config = enxlog_sink_config_create();
    config->sinks = NULL;

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

    // Transform the filter config
    config->filter = enxlog_filter_config_transform(config->filter_config);

    // Transform the sink config
    config->sinks = enxlog_sink_config_transform(config->sink_config, sink_creation_callback, error_callback);

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
    if (config->filter_config) {
        enxlog_filter_config_destroy(config->filter_config);
    }

    if (config->sink_config) {
        enxlog_sink_config_destroy(config->sink_config);
    }

    if (config->filter) {
        enxlog_filter_config_transform_destroy(config->filter);
    }

    if (config->sinks) {
        enxlog_sink_config_transform_destroy(config->sinks);
    }

   free(config);
}

enum enxlog_loglevel enxlog_config_get_default_loglevel(struct enxlog_config* config)
{
    return config->default_loglevel;
}

const struct enxlog_sink *enxlog_config_get_sinks(struct enxlog_config* config)
{
    return config->sinks;
}

const struct enxlog_filter *enxlog_config_get_filter(struct enxlog_config* config)
{
    return config->filter;
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

                            // Options
                            if (strcmp(label, "options") == 0) {
                                if (!enxlog_config_parse_generic_mapping(
                                        config,
                                        parser,
                                        enxlog_config_parse_section_options,
                                        config)) {

                                    parse_state = PARSE_STATE_ERROR;
                                }
                            }

                            // Sink
                            else if (strcmp(label, "sink") == 0) {

                                struct enxlog_sink_parameters *sink_parameters = enxlog_sink_parameters_create();

                                if (enxlog_config_parse_generic_mapping(
                                    config,
                                    parser,
                                    enxlog_config_parse_section_sink,
                                    sink_parameters)) {
                                    
                                    enxlog_sink_config_append(config->sink_config, sink_parameters);

                                } else {
                                    enxlog_sink_parameters_destroy(sink_parameters);
                                    parse_state = PARSE_STATE_ERROR;
                                }
                            }
                            // Filter
                            else if (strcmp(label, "filter") == 0) {
                                if (!enxlog_config_parse_generic_mapping(
                                        config,
                                        parser,
                                        enxlog_config_parse_section_filter,
                                        config)) {

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

static void enxlog_config_parse_section_options(void *ctx, const char *key, const char *value)
{
    struct enxlog_config *config = (struct enxlog_config *)ctx;

    if (strcmp(key, "default_loglevel") == 0) {
        config->default_loglevel = enxlog_config_parse_loglevel(value);
    }
}

static void enxlog_config_parse_section_sink(void *ctx, const char *key, const char *value)
{
    struct enxlog_sink_parameters *sink_parameters = (struct enxlog_sink_parameters *)ctx;
    enxlog_sink_parameters_add(sink_parameters, key, value);
}

static void enxlog_config_parse_section_filter(void *ctx, const char *key, const char *value)
{
    struct enxlog_config *config = (struct enxlog_config *)ctx;
    enxlog_filter_config_append(config->filter_config, key, enxlog_config_parse_loglevel(value));
}


static bool enxlog_config_parse_generic_mapping(
    struct enxlog_config *config,
    yaml_parser_t *parser,
    enxlog_config_parse_mapping_callback_t callback,
    void *context)
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
                            callback(context, key, value);
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
