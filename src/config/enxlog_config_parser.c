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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


struct enxlog_config
{
    struct enxlog_sink* sinks;
    struct enxlog_filter_entry *filter_list;
};

enum token_type
{
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_SEPARATOR,
    TOKEN_ASSIGNMENT,
    TOKEN_SECTION,
    TOKEN_COMMENT
};

enum parse_state_identifier
{
    PARSE_STATE_SECTION_NONE,
    PARSE_STATE_SECTION_SINK,
    PARSE_STATE_SECTION_LOGGER,
    PARSE_STATE_ERROR
};


struct enxlog_config_tokenizer
{
    const char* input;
    const char* ptr;
};

struct enxlog_config_tokenizer_token
{
    int token_type;
    const char* start;
    const char* end;
};


struct enxlog_config_parse_state
{
    enum parse_state_identifier state;

    struct enxlog_config *config;
    struct enxlog_config_sink_parameter_list sink_parameter_list;
    struct enxlog_config_sink_parameters* sink_parameters;
    int line_number;
};


static void enxlog_filter_destroy_recursive(const struct enxlog_filter_entry* filter_entry);

static struct enxlog_config_parse_state* enxlog_config_create_parse_state();
static void enxlog_config_destroy_parse_state(struct enxlog_config_parse_state* parse_state);

static void enxlog_config_parse_section_none(
    struct enxlog_config_parse_state* parse_state,
    const char* line,
    enxlog_config_parser_error_callback_t error_callback);

static void enxlog_config_parse_section_sink(
    struct enxlog_config_parse_state* parse_state,
    const char* line,
    enxlog_config_parser_error_callback_t error_callback);

static void enxlog_config_parse_section_logger(
    struct enxlog_config_parse_state* parse_state,
    const char* line,
    enxlog_config_parser_error_callback_t error_callback);

static struct enxlog_filter_entry* enxlog_filter_allocate_empty_entry();
static struct enxlog_filter_entry* enxlog_filter_find_entry(struct enxlog_filter_entry* parent, const char* path);
static struct enxlog_filter_entry* enxlog_filter_append_child_entry(struct enxlog_filter_entry* parent, const char* path, enum enxlog_loglevel loglevel);
static enum enxlog_loglevel enxlog_config_parse_loglevel(const char* name);

static void enxlog_config_tokenizer_init(
    struct enxlog_config_tokenizer* tokenizer,
    const char* input);

static int enxlog_config_tokenizer_get_next_token(
    int line,
    struct enxlog_config_tokenizer* tokenizer,
    struct enxlog_config_tokenizer_token* token,
    enxlog_config_parser_error_callback_t error_callback);

static char* enxlog_config_token_to_string(const struct enxlog_config_tokenizer_token* token);


struct enxlog_config* enxlog_config_parse(
    const char* path,
    enxlog_config_parser_sink_creation_callback_t sink_creation_callback,
    enxlog_config_parser_error_callback_t error_callback)
{
    char* line = NULL;
    size_t line_length = 0;


    // Open the input file
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        return NULL;
    }

    // Create the parse state
    struct enxlog_config_parse_state* parse_state = enxlog_config_create_parse_state();

    // Process lines
    while (getline(&line, &line_length, file) != -1) {

        switch (parse_state->state) {

            case PARSE_STATE_SECTION_NONE: {
                enxlog_config_parse_section_none(parse_state, line, error_callback);
            } break;

            case PARSE_STATE_SECTION_SINK: {
                enxlog_config_parse_section_sink(parse_state, line, error_callback);
            } break;

            case PARSE_STATE_SECTION_LOGGER: {
                enxlog_config_parse_section_logger(parse_state, line, error_callback);
            } break;

            default: break;
        }

        free(line);
        line_length = 0;
        parse_state->line_number++;

        if (parse_state->state == PARSE_STATE_ERROR) {
            break;
        }
    }

    // Append dangling sink parameter
    if (parse_state->sink_parameters) {
        enxlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
        parse_state->sink_parameters = NULL;
    }

    // Close the file
    fclose(file);


    // If a parse error has occurred, destroy the parse state and exit
    if (parse_state->state == PARSE_STATE_ERROR) {
        enxlog_config_destroy_parse_state(parse_state);
        return NULL;
    }


    // Attempt to create the sinks
    size_t sink_count = enxlog_config_sink_parameter_list_get_count(&parse_state->sink_parameter_list);
    parse_state->config->sinks = malloc(sizeof(struct enxlog_sink) * (sink_count + 1));
    parse_state->config->sinks[sink_count].fn_output = NULL;
    parse_state->config->sinks[sink_count].fn_shutdown = NULL;
    parse_state->config->sinks[sink_count].context = NULL;

    bool sinks_created = true;

    for (size_t i=0; i < sink_count; ++i) {
        struct enxlog_config_sink_parameters* parameters = enxlog_config_sink_parameter_list_get_at(&parse_state->sink_parameter_list, i);
        if (!enxlog_config_sink_factory_create_sink(&parse_state->config->sinks[i], parameters, sink_creation_callback, error_callback)) {

            // Destroy the created sinks
            for (size_t j=0; j < i; ++j) {
                if (parse_state->config->sinks[j].fn_shutdown) {
                    parse_state->config->sinks[j].fn_shutdown(parse_state->config->sinks[j].context);
                }
            }
            sinks_created = false;
            break;
        }
    }

    // If any sink creation failed, destroy the parse state and exit
    if (!sinks_created) {
        enxlog_config_destroy_parse_state(parse_state);
        return NULL;
    }


    // Detach the config, destroy the parse state, and exit
    struct enxlog_config* config = parse_state->config;
    parse_state->config = NULL;
    enxlog_config_destroy_parse_state(parse_state);

    return config;
}

void enxlog_config_destroy(struct enxlog_config* config)
{
    // Destroy the filter list
    enxlog_filter_destroy_recursive(config->filter_list);
    free(config->filter_list);

    // Free the sinks
    struct enxlog_sink* sink = config->sinks;
    while (sink->fn_output) {
        if (sink->fn_shutdown) {
            sink->fn_shutdown(sink->context);
        }
        sink++;
    }

    // Free the sink list
    if (config->sinks) {
        free(config->sinks);
    }

    // Free the configuration object
    free(config);
}

enum enxlog_loglevel enxlog_config_get_default_loglevel(struct enxlog_config* config)
{
    return config->filter_list->loglevel;
}

const struct enxlog_sink *enxlog_config_get_sinks(struct enxlog_config* config)
{
    return config->sinks;
}

const struct enxlog_filter_entry *enxlog_config_get_filter_list(struct enxlog_config* config)
{
    return config->filter_list->children;
}

static struct enxlog_config_parse_state* enxlog_config_create_parse_state()
{
    struct enxlog_config_parse_state *parse_state = malloc(sizeof(struct enxlog_config_parse_state));

    parse_state->config = malloc(sizeof(struct enxlog_config));

    parse_state->config->filter_list = (struct enxlog_filter_entry *)malloc(sizeof(struct enxlog_filter_entry));
    parse_state->config->filter_list->loglevel = LOGLEVEL_NONE;
    parse_state->config->filter_list->path = strdup("");
    parse_state->config->filter_list->children = enxlog_filter_allocate_empty_entry();

    parse_state->state = PARSE_STATE_SECTION_NONE;
    parse_state->line_number = 0;

    enxlog_config_sink_parameter_list_init(&parse_state->sink_parameter_list);
    parse_state->sink_parameters = NULL;

    return parse_state;
}

static void enxlog_config_destroy_parse_state(struct enxlog_config_parse_state* parse_state)
{
    // Destroy the sink parameter list
    enxlog_config_sink_parameter_list_destroy(&parse_state->sink_parameter_list);

    // Destroy any dangling sink parameters
    if (parse_state->sink_parameters) {
        enxlog_config_sink_parameters_destroy(parse_state->sink_parameters);
    }

    // Destroy the config
    if (parse_state->config) {
        enxlog_config_destroy(parse_state->config);
    }

    // Free the parse state
    free(parse_state);
}

static void enxlog_config_parse_section_none(
    struct enxlog_config_parse_state* parse_state,
    const char* line,
    enxlog_config_parser_error_callback_t error_callback)
{

    struct enxlog_config_tokenizer tokenizer;
    enxlog_config_tokenizer_init(&tokenizer, line);

    struct enxlog_config_tokenizer_token token;
    int result = enxlog_config_tokenizer_get_next_token(parse_state->line_number, &tokenizer, &token, error_callback);

    if (result == -1) {
        parse_state->state = PARSE_STATE_ERROR;

    } else if (result == 0) {
        // Ignore empty line

    } else {

        switch (token.token_type) {

            case TOKEN_SECTION: {

                char *name = enxlog_config_token_to_string(&token);
                if (strcmp(name, "sink") == 0) {
                    free(name);

                    if (parse_state->sink_parameters) {
                        enxlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
                    }

                    parse_state->sink_parameters = enxlog_config_sink_parameters_create();

                    parse_state->state = PARSE_STATE_SECTION_SINK;

                } else if (strcmp(name, "logger") == 0) {
                    free(name);
                    parse_state->state = PARSE_STATE_SECTION_LOGGER;

                } else {
                    error_callback(parse_state->line_number, (int) (tokenizer.ptr - tokenizer.input), "Unknown section");
                    parse_state->state = PARSE_STATE_ERROR;
                }
            } break;

            case TOKEN_COMMENT: break;

            default: {
                error_callback(parse_state->line_number, (int) (tokenizer.ptr - tokenizer.input), "Expected SECTION");
                parse_state->state = PARSE_STATE_ERROR;
            } break;
        }
    }
}

static void enxlog_config_parse_section_sink(
    struct enxlog_config_parse_state* parse_state,
    const char* line, enxlog_config_parser_error_callback_t error_callback)
{
    enum parse_substate
    {
        PARSE_SUBSTATE_START,
        PARSE_SUBSTATE_NAME,
        PARSE_SUBSTATE_ASSIGNMENT,
        PARSE_SUBSTATE_VALUE,
        PARSE_SUBSTATE_DONE
    };


    char* parameter_name = NULL;
    char* parameter_value = NULL;


    // Initialize the tokenizer
    struct enxlog_config_tokenizer tokenizer;
    enxlog_config_tokenizer_init(&tokenizer, line);


    enum parse_substate parse_substate = PARSE_SUBSTATE_START;

    while (parse_substate != PARSE_SUBSTATE_DONE) {
        struct enxlog_config_tokenizer_token token;
        int result;

        // Consume the next token
        result = enxlog_config_tokenizer_get_next_token(parse_state->line_number, &tokenizer, &token, error_callback);
        if (result == -1) {
            parse_substate = PARSE_SUBSTATE_DONE;

        } else if (result == 0) {

            if (parse_substate != PARSE_SUBSTATE_START) {
                error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected input, found EOL");
                parse_substate = PARSE_SUBSTATE_DONE;
                parse_state->state = PARSE_STATE_ERROR;

            } else {
                parse_substate = PARSE_SUBSTATE_DONE;
            }

        } else {

            switch (parse_substate) {

                case PARSE_SUBSTATE_START: {

                    switch (token.token_type) {

                        case TOKEN_SECTION: {

                            char *name = enxlog_config_token_to_string(&token);
                            if (strcmp(name, "sink") == 0) {
                                free(name);

                                if (parse_state->sink_parameters) {
                                    enxlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
                                }
                                parse_state->sink_parameters = enxlog_config_sink_parameters_create();

                                parse_state->state = PARSE_STATE_SECTION_SINK;
                                parse_substate = PARSE_SUBSTATE_DONE;

                            } else if (strcmp(name, "logger") == 0) {
                                free(name);
                                parse_state->state = PARSE_STATE_SECTION_LOGGER;
                                parse_substate = PARSE_SUBSTATE_DONE;
                            } else {

                                error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Unknown section");
                                parse_state->state = PARSE_STATE_ERROR;
                                parse_substate = PARSE_SUBSTATE_DONE;
                            }
                        } break;

                        case TOKEN_IDENTIFIER: {
                            parameter_name = enxlog_config_token_to_string(&token);
                            parse_substate = PARSE_SUBSTATE_ASSIGNMENT;
                        } break;

                        case TOKEN_COMMENT: {
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;

                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }

                } break;

                case PARSE_SUBSTATE_ASSIGNMENT: {

                    switch (token.token_type) {

                        case TOKEN_ASSIGNMENT: {
                            parse_substate = PARSE_SUBSTATE_VALUE;

                        } break;
                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected '='");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }

                } break;

                case PARSE_SUBSTATE_VALUE: {
                    switch (token.token_type) {

                        case TOKEN_STRING: {
                            parameter_value = enxlog_config_token_to_string(&token);
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;

                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }

                } break;

            }
        }
    }

    if ((parse_state->state != PARSE_STATE_ERROR) && (parameter_name != NULL) && (parameter_value != NULL)) {
        enxlog_config_sink_parameters_add(parse_state->sink_parameters, parameter_name, parameter_value);
    }

    if (parameter_name) {
        free(parameter_name);
    }

    if (parameter_value) {
        free(parameter_value);
    }
}

static void enxlog_config_parse_section_logger(
    struct enxlog_config_parse_state* parse_state,
    const char* line, enxlog_config_parser_error_callback_t error_callback)
{
    enum parse_substate
    {
        PARSE_SUBSTATE_START,
        PARSE_SUBSTATE_PATHNAME,
        PARSE_SUBSTATE_SEPARATOR,
        PARSE_SUBSTATE_IDENTIFIER,
        PARSE_SUBSTATE_DONE
    };

    // Reset the parent pointer
    struct enxlog_filter_entry* parent = parse_state->config->filter_list;

    // Initialize the tokenizer
    struct enxlog_config_tokenizer tokenizer;
    enxlog_config_tokenizer_init(&tokenizer, line);


    enum parse_substate parse_substate = PARSE_SUBSTATE_START;

    while (parse_substate != PARSE_SUBSTATE_DONE) {
        struct enxlog_config_tokenizer_token token;
        int result;

        // Consume the next token
        result = enxlog_config_tokenizer_get_next_token(parse_state->line_number, &tokenizer, &token, error_callback);
        if (result == -1) {
            parse_substate = PARSE_SUBSTATE_DONE;

        } else if (result == 0) {

            if (parse_substate != PARSE_SUBSTATE_START) {
                error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected input, found EOL");
                parse_substate = PARSE_SUBSTATE_DONE;
                parse_state->state = PARSE_STATE_ERROR;

            } else {
                parse_substate = PARSE_SUBSTATE_DONE;
            }

        } else {

            switch (parse_substate) {

                case PARSE_SUBSTATE_START: {

                    switch (token.token_type) {

                        case TOKEN_SECTION: {

                            char *name = enxlog_config_token_to_string(&token);
                            if (strcmp(name, "sink") == 0) {
                                free(name);

                                if (parse_state->sink_parameters) {
                                    enxlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
                                }
                                parse_state->sink_parameters = enxlog_config_sink_parameters_create();

                                parse_state->state = PARSE_STATE_SECTION_SINK;
                                parse_substate = PARSE_SUBSTATE_DONE;

                            } else if (strcmp(name, "logger") == 0) {
                                free(name);
                                parse_state->state = PARSE_STATE_SECTION_LOGGER;
                                parse_substate = PARSE_SUBSTATE_DONE;

                            } else {
                                error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Unknown section");
                                parse_state->state = PARSE_STATE_ERROR;
                                parse_substate = PARSE_SUBSTATE_DONE;
                            }
                        } break;

                        case TOKEN_STRING: {

                            // Find the entry in the current parent
                            char* path = enxlog_config_token_to_string(&token);
                            struct enxlog_filter_entry* entry = enxlog_filter_find_entry(parent, path);
                            if (entry) {
                                parent = entry;
                                free(path);
                            } else {
                                parent = enxlog_filter_append_child_entry(parent, path, LOGLEVEL_NONE);
                            }

                            parse_substate = PARSE_SUBSTATE_SEPARATOR;
                        } break;

                        case TOKEN_COMMENT: {
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;

                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }

                } break;

                case PARSE_SUBSTATE_PATHNAME: {

                    switch (token.token_type) {

                        case TOKEN_STRING: {

                            // Find the entry in the current parent
                            char* path = enxlog_config_token_to_string(&token);
                            struct enxlog_filter_entry* entry = enxlog_filter_find_entry(parent, path);
                            if (entry) {
                                parent = entry;
                                free(path);
                            } else {
                                parent = enxlog_filter_append_child_entry(parent, path, LOGLEVEL_NONE);
                            }

                            parse_substate = PARSE_SUBSTATE_SEPARATOR;
                        } break;

                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }

                } break;

                case PARSE_SUBSTATE_SEPARATOR: {
                    switch (token.token_type) {

                        case TOKEN_SEPARATOR: {
                            parse_substate = PARSE_SUBSTATE_PATHNAME;
                        } break;

                        case TOKEN_ASSIGNMENT: {
                            parse_substate = PARSE_SUBSTATE_IDENTIFIER;
                        } break;

                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected '.' or '='");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }

                } break;

                case PARSE_SUBSTATE_IDENTIFIER: {

                    switch (token.token_type) {
                        case TOKEN_IDENTIFIER: {

                            char* identifier = enxlog_config_token_to_string(&token);
                            parent->loglevel = enxlog_config_parse_loglevel(identifier);
                            free(identifier);
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;

                        default: {
                            error_callback(parse_state->line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected IDENTIFIER");
                            parse_state->state = PARSE_STATE_ERROR;
                            parse_substate = PARSE_SUBSTATE_DONE;
                        } break;
                    }
                } break;
            }
        }
    }
}

static void enxlog_filter_destroy_recursive(const struct enxlog_filter_entry* config)
{
    // Recursively deallocate the children
    const struct enxlog_filter_entry* index = config->children;
    while (index->path) {
        enxlog_filter_destroy_recursive(index);
        index++;
    }

    // Free the object pointers
    free((char *)config->path);
    free((void *)config->children);
}

static struct enxlog_filter_entry* enxlog_filter_allocate_empty_entry()
{
    struct enxlog_filter_entry* node = malloc(sizeof(struct enxlog_filter_entry));
    node->loglevel = LOGLEVEL_NONE;
    node->path = NULL;
    node->children = NULL;

    return node;
}

static struct enxlog_filter_entry* enxlog_filter_find_entry(struct enxlog_filter_entry* parent, const char* path)
{
    const struct enxlog_filter_entry* search = parent->children;
    while ((search->path) && (strcmp(search->path, path) != 0)) {
        search++;
    }

    if (search->path == NULL) {
        search = NULL;
    }

    return (struct enxlog_filter_entry*)search;
}

static struct enxlog_filter_entry* enxlog_filter_append_child_entry(struct enxlog_filter_entry* parent, const char* path, enum enxlog_loglevel loglevel)
{
    const struct enxlog_filter_entry* search = parent->children;
    while (search->path) {
        search++;
    }

    size_t entries = (search - parent->children) + 1;
    parent->children = (struct enxlog_filter_entry*)realloc((void*)parent->children, (entries + 1) * sizeof(struct enxlog_filter_entry));

    memcpy((void*)(parent->children + entries), parent->children + entries - 1, sizeof(struct enxlog_filter_entry));

    struct enxlog_filter_entry* child = (struct enxlog_filter_entry*)(parent->children + (entries - 1));
    child->path = path;
    child->loglevel = loglevel;
    child->children = enxlog_filter_allocate_empty_entry();

    return child;
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

static void enxlog_config_tokenizer_init(
    struct enxlog_config_tokenizer* tokenizer,
    const char* input)
{
    tokenizer->input = input;
    tokenizer->ptr = input;
}

static int enxlog_config_tokenizer_get_next_token(
    int line,
    struct enxlog_config_tokenizer* tokenizer,
    struct enxlog_config_tokenizer_token* token,
    enxlog_config_parser_error_callback_t error_callback)
{
    // Skip over whitespace
    while (*tokenizer->ptr && isspace(*tokenizer->ptr)) {
        tokenizer->ptr++;
    }

    // End of stream
    if (*tokenizer->ptr == 0) {
        return 0;
    }

    // Determine token type
    if (isalpha(*tokenizer->ptr)) {
        token->start = tokenizer->ptr++;
        while (*tokenizer->ptr && isalpha(*tokenizer->ptr)) {
            tokenizer->ptr++;
        }
        token->end = tokenizer->ptr - 1;
        token->token_type = TOKEN_IDENTIFIER;

        return 1;
    }

    if (*tokenizer->ptr == '\"') {
        token->start = tokenizer->ptr++;
        while (*tokenizer->ptr && *tokenizer->ptr != '\"') {
            tokenizer->ptr++;
        }

        if (*tokenizer->ptr == 0) {
            error_callback(line, (int)(tokenizer->ptr - tokenizer->input), "Expected \'\"\', found EOL");
            return -1;
        } else {
            tokenizer->ptr++;
        }

        token->end = tokenizer->ptr - 1;
        token->token_type = TOKEN_STRING;

        return 1;
    }

    if (*tokenizer->ptr == '[') {
        token->start = tokenizer->ptr++;
        while (*tokenizer->ptr && *tokenizer->ptr != ']') {
            tokenizer->ptr++;
        }

        if (*tokenizer->ptr == 0) {
            error_callback(line, (int)(tokenizer->ptr - tokenizer->input), "Expected \']\', found EOL");
            return -1;
        } else {
            tokenizer->ptr++;
        }

        token->end = tokenizer->ptr - 1;
        token->token_type = TOKEN_SECTION;

        return 1;
    }

    if (*tokenizer->ptr == '.') {
        token->start = token->end = tokenizer->ptr++;
        token->token_type = TOKEN_SEPARATOR;

        return 1;
    }

    if (*tokenizer->ptr == '=') {
        token->start = token->end = tokenizer->ptr++;
        token->token_type = TOKEN_ASSIGNMENT;

        return 1;
    }

    if (*tokenizer->ptr == '#') {
        token->start = token->end = tokenizer->ptr++;
        token->token_type = TOKEN_COMMENT;

        return 1;
    }

    error_callback(line, (int)(tokenizer->ptr - tokenizer->input), "Unexpected character");
    return -1;
}

static char* enxlog_config_token_to_string(const struct enxlog_config_tokenizer_token* token)
{
    char* result = NULL;

    switch (token->token_type) {

        case TOKEN_STRING: {
            size_t length = (token->end - token->start);
            result = malloc(length);
            memcpy(result, token->start + 1, length - 1);
            result[length-1] = 0;
        } break;

        case TOKEN_SECTION: {
            size_t length = (token->end - token->start);
            result = malloc(length);
            memcpy(result, token->start + 1, length - 1);
            result[length-1] = 0;
        } break;

        case TOKEN_IDENTIFIER: {
            size_t length = (token->end - token->start) + 2;
            result = malloc(length);
            memcpy(result, token->start, length - 1);
            result[length-1] = 0;
        } break;

        default: break;
    }

    return result;
}
