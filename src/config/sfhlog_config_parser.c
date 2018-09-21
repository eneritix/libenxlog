/*
	Copyright (c) 2018 Eneritix (Pty) Ltd

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

#include <sfhlog/config/sfhlog_config_parser.h>
#include <sfhlog/config/sfhlog_config_sink_parameters.h>
#include <sfhlog/config/sfhlog_config_sink_factory.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>



enum token_type { TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_SEPARATOR, TOKEN_ASSIGNMENT, TOKEN_SECTION, TOKEN_COMMENT };


enum parse_state
{
	PARSE_STATE_SECTION_NONE,
	PARSE_STATE_SECTION_SINK,
	PARSE_STATE_SECTION_LOGGER,
	PARSE_STATE_ERROR
};


struct sfhlog_config_tokenizer
{
	const char* input;
	const char* ptr;
};

struct sfhlog_config_tokenizer_token
{
	int token_type;
	const char* start;
	const char* end;
};


struct sfhlog_config_parse_state
{
	struct sfhlog_config* config;
	struct sfhlog_config_sink_parameter_list sink_parameter_list;
	struct sfhlog_config_sink_parameters* sink_parameters;

};


static enum parse_state sfhlog_config_parse_section_sink(struct sfhlog_config_parse_state* parse_state, const char* line, int line_number, sfhlog_config_parser_error_callback_t error_callback);
static enum parse_state sfhlog_config_parse_section_logger(struct sfhlog_config_parse_state* parse_state, const char* line, int line_number, sfhlog_config_parser_error_callback_t error_callback);

static struct sfhlog_config_node* sfhlog_config_allocate_empty_entry();
static struct sfhlog_config_node* sfhlog_config_find_entry(struct sfhlog_config_node* parent, const char* path);
static struct sfhlog_config_node* sfhlog_config_append_child_entry(struct sfhlog_config_node* parent, const char* path, enum sfhlog_severity severity);
static enum sfhlog_severity sfhlog_config_parse_severity(const char* name);



static void sfhlog_config_destroy_recursive(struct sfhlog_config_node* config);

static void sfhlog_config_tokenizer_init(
	struct sfhlog_config_tokenizer* tokenizer,
	const char* input);

static int sfhlog_config_tokenizer_get_next_token(
	int line,
	struct sfhlog_config_tokenizer* tokenizer,
	struct sfhlog_config_tokenizer_token* token,
	sfhlog_config_parser_error_callback_t error_callback);

static char* sfhlog_config_token_to_string(const struct sfhlog_config_tokenizer_token* token);


struct sfhlog_config* sfhlog_config_parse(
	const char* path,
	sfhlog_config_parser_sink_creation_callback_t sink_creation_callback,
	sfhlog_config_parser_error_callback_t error_callback)
{
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		return NULL;
	}


	struct sfhlog_config_parse_state* parse_state = malloc(sizeof(struct sfhlog_config_parse_state));

	parse_state->config = malloc(sizeof(struct sfhlog_config));
	parse_state->config->root = malloc(sizeof(struct sfhlog_config_node));
	parse_state->config->sinks = NULL;

	parse_state->config->root->path = strdup("");
	parse_state->config->root->severity = LOG_SEVERITY_NONE;
	parse_state->config->root->children = sfhlog_config_allocate_empty_entry();

	sfhlog_config_sink_parameter_list_init(&parse_state->sink_parameter_list);
	parse_state->sink_parameters = NULL;


	char* line = NULL;
	size_t line_length = 0;



	enum parse_state state = PARSE_STATE_SECTION_NONE;
	int line_number = 0;

	while (getline(&line, &line_length, file) != -1) {


		switch (state) {

			case PARSE_STATE_SECTION_NONE: {

				struct sfhlog_config_tokenizer tokenizer;
				sfhlog_config_tokenizer_init(&tokenizer, line);

				struct sfhlog_config_tokenizer_token token;
				int result = sfhlog_config_tokenizer_get_next_token(line_number, &tokenizer, &token, error_callback);

				if (result == -1) {
					state = PARSE_STATE_ERROR;

				} else {

					switch (token.token_type) {

						case TOKEN_SECTION: {

							char *name = sfhlog_config_token_to_string(&token);
							if (strcmp(name, "sink") == 0) {
								free(name);

								if (parse_state->sink_parameters) {
									sfhlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
								}
								parse_state->sink_parameters = sfhlog_config_sink_parameters_create();

								state = PARSE_STATE_SECTION_SINK;

							} else if (strcmp(name, "logger") == 0) {
								free(name);
								state = PARSE_STATE_SECTION_LOGGER;

							} else {

								error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Unknown section");
								state = PARSE_STATE_ERROR;
							}
						} break;

						case TOKEN_COMMENT: break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected SECTION");
							state = PARSE_STATE_ERROR;
						} break;

					} break;
				}
			} break;

			case PARSE_STATE_SECTION_SINK: {
				state = sfhlog_config_parse_section_sink(parse_state, line, line_number, error_callback);
			} break;

			case PARSE_STATE_SECTION_LOGGER: {
				state = sfhlog_config_parse_section_logger(parse_state, line, line_number, error_callback);
			} break;

			default: break;
		}

		free(line);
		line_length = 0;
		line_number++;

		if (state == PARSE_STATE_ERROR) {
			break;
		}

	}

	// Append dangling sink parameter
	if (parse_state->sink_parameters) {
		sfhlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
	}

	// Close the file
	fclose(file);


	// If a parse error has occured, destroy all the objects and exit
	if (state == PARSE_STATE_ERROR) {
		sfhlog_config_sink_parameter_list_destroy(&parse_state->sink_parameter_list);
		sfhlog_config_destroy(parse_state->config);

		free(parse_state);

		return NULL;
	}


	// Attempt to create the sinks
	size_t sink_count = sfhlog_config_sink_parameter_list_get_count(&parse_state->sink_parameter_list);
	parse_state->config->sinks = malloc(sizeof(struct sfhlog_sink) * (sink_count + 1));
	parse_state->config->sinks[sink_count].output_function = NULL;
	parse_state->config->sinks[sink_count].shutdown_function = NULL;
	parse_state->config->sinks[sink_count].context = NULL;

	bool sinks_created = true;

	for (size_t i=0; i < sink_count; ++i) {
		struct sfhlog_config_sink_parameters* parameters = sfhlog_config_sink_parameter_list_get_at(&parse_state->sink_parameter_list, i);
		if (!sfhlog_config_sink_factory_create_sink(&parse_state->config->sinks[i], parameters, sink_creation_callback, error_callback)) {

			// Destroy the created sinks
			for (size_t j=0; j < i; ++j) {
				if (parse_state->config->sinks[j].shutdown_function) {
					parse_state->config->sinks[j].shutdown_function(parse_state->config->sinks[j].context);
				}
			}
			sinks_created = false;
			break;
		}
	}

	if (!sinks_created) {
		sfhlog_config_sink_parameter_list_destroy(&parse_state->sink_parameter_list);
		sfhlog_config_destroy(parse_state->config);
		free(parse_state);
		return NULL;
	}


	sfhlog_config_sink_parameter_list_destroy(&parse_state->sink_parameter_list);
	struct sfhlog_config* config = parse_state->config;
	free(parse_state);

	return config;
}

void sfhlog_config_destroy(struct sfhlog_config* config)
{
	// Destroy the configuration entries
	sfhlog_config_destroy_recursive(config->root);
	free(config->root);

	// Free the sinks
	struct sfhlog_sink* sink = config->sinks;
	while (sink->output_function) {
		if (sink->shutdown_function) {
			sink->shutdown_function(sink->context);
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

static enum parse_state sfhlog_config_parse_section_sink(struct sfhlog_config_parse_state* parse_state, const char* line, int line_number, sfhlog_config_parser_error_callback_t error_callback)
{
	enum parse_substate
	{
		PARSE_SUBSTATE_START,
		PARSE_SUBSTATE_NAME,
		PARSE_SUBSTATE_ASSIGNMENT,
		PARSE_SUBSTATE_VALUE,
		PARSE_SUBSTATE_DONE
	};

	enum parse_state parent_state = PARSE_STATE_SECTION_SINK;


	char* parameter_name = NULL;
	char* parameter_value = NULL;


	// Initialize the tokenizer
	struct sfhlog_config_tokenizer tokenizer;
	sfhlog_config_tokenizer_init(&tokenizer, line);


	enum parse_substate state = PARSE_SUBSTATE_START;

	while (state != PARSE_SUBSTATE_DONE) {
		struct sfhlog_config_tokenizer_token token;
		int result;

		// Consume the next token
		result = sfhlog_config_tokenizer_get_next_token(line_number, &tokenizer, &token, error_callback);
		if (result == -1) {
			state = PARSE_SUBSTATE_DONE;

		} else if (result == 0) {

			if (state != PARSE_SUBSTATE_START) {
				error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected input, found EOL");
				state = PARSE_SUBSTATE_DONE;
				parent_state = PARSE_STATE_ERROR;

			} else {
				state = PARSE_SUBSTATE_DONE;
			}

		} else {


			switch (state) {

				case PARSE_SUBSTATE_START: {

					switch (token.token_type) {

						case TOKEN_SECTION: {

							char *name = sfhlog_config_token_to_string(&token);
							if (strcmp(name, "sink") == 0) {
								free(name);

								if (parse_state->sink_parameters) {
									sfhlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
								}
								parse_state->sink_parameters = sfhlog_config_sink_parameters_create();

								parent_state = PARSE_STATE_SECTION_SINK;
								state = PARSE_SUBSTATE_DONE;

							} else if (strcmp(name, "logger") == 0) {
								free(name);
								parent_state = PARSE_STATE_SECTION_LOGGER;
								state = PARSE_SUBSTATE_DONE;
							} else {

								error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Unknown section");
								parent_state = PARSE_STATE_ERROR;
								state = PARSE_SUBSTATE_DONE;
							}
						} break;

						case TOKEN_IDENTIFIER: {
							parameter_name = sfhlog_config_token_to_string(&token);
							state = PARSE_SUBSTATE_ASSIGNMENT;
						} break;

						case TOKEN_COMMENT: {
							state = PARSE_SUBSTATE_DONE;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;
						} break;
					}

				} break;

				case PARSE_SUBSTATE_ASSIGNMENT: {

					switch (token.token_type) {

						case TOKEN_ASSIGNMENT: {
							state = PARSE_SUBSTATE_VALUE;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected '='");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;

						} break;
					}

				} break;

				case PARSE_SUBSTATE_VALUE: {
					switch (token.token_type) {

						case TOKEN_STRING: {
							parameter_value = sfhlog_config_token_to_string(&token);
							state = PARSE_SUBSTATE_DONE;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;
						} break;
					}

				} break;

			}
		}
	}

	if ((parent_state != PARSE_STATE_ERROR) && (parameter_name != NULL) && (parameter_value != NULL)) {
		sfhlog_config_sink_parameters_add(parse_state->sink_parameters, parameter_name, parameter_value);
	}

	if (parameter_name) {
		free(parameter_name);
	}

	if (parameter_value) {
		free(parameter_value);
	}


	return parent_state;

}

static enum parse_state sfhlog_config_parse_section_logger(struct sfhlog_config_parse_state* parse_state, const char* line, int line_number, sfhlog_config_parser_error_callback_t error_callback)
{
	enum parse_substate
	{
		PARSE_SUBSTATE_START,
		PARSE_SUBSTATE_PATHNAME,
		PARSE_SUBSTATE_SEPARATOR,
		PARSE_SUBSTATE_IDENTIFIER,
		PARSE_SUBSTATE_DONE
	};

	enum parse_state parent_state = PARSE_STATE_SECTION_LOGGER;

	// Reset the parent pointer
	struct sfhlog_config_node* parent = parse_state->config->root;

	// Initialize the tokenizer
	struct sfhlog_config_tokenizer tokenizer;
	sfhlog_config_tokenizer_init(&tokenizer, line);


	enum parse_substate state = PARSE_SUBSTATE_START;

	while (state != PARSE_SUBSTATE_DONE) {
		struct sfhlog_config_tokenizer_token token;
		int result;

		// Consume the next token
		result = sfhlog_config_tokenizer_get_next_token(line_number, &tokenizer, &token, error_callback);
		if (result == -1) {
			state = PARSE_SUBSTATE_DONE;

		} else if (result == 0) {

			if (state != PARSE_SUBSTATE_START) {
				error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected input, found EOL");
				state = PARSE_SUBSTATE_DONE;
				parent_state = PARSE_STATE_ERROR;

			} else {
				state = PARSE_SUBSTATE_DONE;
			}

		} else {


			switch (state) {

				case PARSE_SUBSTATE_START: {

					switch (token.token_type) {

						case TOKEN_SECTION: {

							char *name = sfhlog_config_token_to_string(&token);
							if (strcmp(name, "sink") == 0) {
								free(name);

								if (parse_state->sink_parameters) {
									sfhlog_config_sink_parameter_list_add(&parse_state->sink_parameter_list, parse_state->sink_parameters);
								}
								parse_state->sink_parameters = sfhlog_config_sink_parameters_create();

								parent_state = PARSE_STATE_SECTION_SINK;
								state = PARSE_SUBSTATE_DONE;

							} else if (strcmp(name, "logger") == 0) {
								free(name);
								parent_state = PARSE_STATE_SECTION_LOGGER;
								state = PARSE_SUBSTATE_DONE;
							} else {

								error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Unknown section");
								parent_state = PARSE_STATE_ERROR;
								state = PARSE_SUBSTATE_DONE;
							}
						} break;

						case TOKEN_STRING: {

							// Find the entry in the current parent
							char* path = sfhlog_config_token_to_string(&token);
							struct sfhlog_config_node* entry = sfhlog_config_find_entry(parent, path);
							if (entry) {
								parent = entry;
								free(path);
							} else {
								parent = sfhlog_config_append_child_entry(parent, path, LOG_SEVERITY_NONE);
							}

							state = PARSE_SUBSTATE_SEPARATOR;
						} break;

						case TOKEN_COMMENT: {
							state = PARSE_SUBSTATE_DONE;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;
						} break;
					}

				} break;

				case PARSE_SUBSTATE_PATHNAME: {

					switch (token.token_type) {

						case TOKEN_STRING: {

							// Find the entry in the current parent
							char* path = sfhlog_config_token_to_string(&token);
							struct sfhlog_config_node* entry = sfhlog_config_find_entry(parent, path);
							if (entry) {
								parent = entry;
								free(path);
							} else {
								parent = sfhlog_config_append_child_entry(parent, path, LOG_SEVERITY_NONE);
							}

							state = PARSE_SUBSTATE_SEPARATOR;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected STRING");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;

						} break;
					}

				} break;

				case PARSE_SUBSTATE_SEPARATOR: {
					switch (token.token_type) {

						case TOKEN_SEPARATOR: {
							state = PARSE_SUBSTATE_PATHNAME;
						} break;

						case TOKEN_ASSIGNMENT: {
							state = PARSE_SUBSTATE_IDENTIFIER;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected '.' or '='");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;
						} break;
					}

				} break;

				case PARSE_SUBSTATE_IDENTIFIER: {

					switch (token.token_type) {
						case TOKEN_IDENTIFIER: {

							char* identifier = sfhlog_config_token_to_string(&token);
							parent->severity = sfhlog_config_parse_severity(identifier);
							free(identifier);
							state = PARSE_SUBSTATE_DONE;
						} break;

						default: {
							error_callback(line_number, (int)(tokenizer.ptr - tokenizer.input), "Expected IDENTIFIER");
							parent_state = PARSE_STATE_ERROR;
							state = PARSE_SUBSTATE_DONE;
						} break;
					}
				} break;
			}
		}
	}

	return parent_state;
}

static void sfhlog_config_destroy_recursive(struct sfhlog_config_node* config)
{
	// Recursively deallocate the children
	struct sfhlog_config_node* index = config->children;
	while (index->path) {
		sfhlog_config_destroy_recursive(index);
		index++;
	}

	// Free the object pointers
	free((char*)config->path);
	free(config->children);
}

static struct sfhlog_config_node* sfhlog_config_allocate_empty_entry()
{
	struct sfhlog_config_node* node = malloc(sizeof(struct sfhlog_config_node));
	node->severity = 0;
	node->path = NULL;
	node->children = NULL;

	return node;
}

static struct sfhlog_config_node* sfhlog_config_find_entry(struct sfhlog_config_node* parent, const char* path)
{
	const struct sfhlog_config_node* search = parent->children;
	while ((search->path) && (strcmp(search->path, path) != 0)) {
		search++;
	}

	if (search->path == NULL) {
		search = NULL;
	}

	return (struct sfhlog_config_node*)search;
}

static struct sfhlog_config_node* sfhlog_config_append_child_entry(struct sfhlog_config_node* parent, const char* path, enum sfhlog_severity severity)
{
	const struct sfhlog_config_node* search = parent->children;
	while (search->path) {
		search++;
	}

	size_t entries = (search - parent->children) + 1;
	parent->children = (struct sfhlog_config_node*)realloc((void*)parent->children, (entries + 1) * sizeof(struct sfhlog_config_node));

	memcpy((void*)(parent->children + entries), parent->children + entries - 1, sizeof(struct sfhlog_config_node));

	struct sfhlog_config_node* child = (parent->children + (entries - 1));
	child->path = path;
	child->severity = severity;
	child->children = sfhlog_config_allocate_empty_entry();

	return child;
}

static enum sfhlog_severity sfhlog_config_parse_severity(const char* name)
{
	if (strcmp(name, "NONE") == 0) {
		return LOG_SEVERITY_NONE;

	} else if (strcmp(name, "ERROR") == 0) {
		return LOG_SEVERITY_ERROR;

	} else if (strcmp(name, "WARN") == 0) {
		return LOG_SEVERITY_WARN;

	} else if (strcmp(name, "INFO") == 0) {
		return LOG_SEVERITY_INFO;

	} else if (strcmp(name, "DEBUG") == 0) {
		return LOG_SEVERITY_DEBUG;
	}

	return LOG_SEVERITY_NONE;
}

static void sfhlog_config_tokenizer_init(
	struct sfhlog_config_tokenizer* tokenizer,
	const char* input)
{
	tokenizer->input = input;
	tokenizer->ptr = input;
}

static int sfhlog_config_tokenizer_get_next_token(
	int line,
	struct sfhlog_config_tokenizer* tokenizer,
	struct sfhlog_config_tokenizer_token* token,
	sfhlog_config_parser_error_callback_t error_callback)
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

static char* sfhlog_config_token_to_string(const struct sfhlog_config_tokenizer_token* token)
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
