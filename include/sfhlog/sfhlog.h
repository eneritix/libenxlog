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

#ifndef SFHLOG_H
#define SFHLOG_H


#include <stdarg.h>
#include <sys/cdefs.h>


__BEGIN_DECLS


enum sfhlog_severity
{
	LOG_SEVERITY_NONE = 0,
	LOG_SEVERITY_ERROR,
	LOG_SEVERITY_WARN,
	LOG_SEVERITY_INFO,
	LOG_SEVERITY_DEBUG
};


struct sfhlog_logger
{
	const char** path;
};


typedef void (*sfhlog_sink_output_function_t)(
	void* context,
	const struct sfhlog_logger* logger,
	enum sfhlog_severity severity,
	const char* func,
	unsigned int line,
	const char* fmt,
	va_list ap);

typedef void (*sfhlog_sink_shutdown_function_t)(void* context);

typedef void (*sfhlog_lock_function_t)(void* context);

struct sfhlog_config_node
{
	const char* path;
	enum sfhlog_severity severity;
	struct sfhlog_config_node* children;
};

struct sfhlog_sink
{
    sfhlog_sink_output_function_t output_function;
    sfhlog_sink_shutdown_function_t shutdown_function;
    void* context;
};

struct sfhlog_lock
{
    sfhlog_lock_function_t lock_function;
    sfhlog_lock_function_t unlock_function;
    void* context;
};

struct sfhlog_config
{
	struct sfhlog_sink* sinks;
	struct sfhlog_lock lock;
	struct sfhlog_config_node* root;
};


#define LOGGER_DECLARE(name, ...) \
static const struct sfhlog_logger* name = (const struct sfhlog_logger []) { \
	{ \
		.path = (const char* []) { \
		__VA_ARGS__ \
		,0 \
		} \
	} \
}

#define SFHLOG_DECLARE_LOCK(_lock_function, _unlock_function, _context) \
    { .lock_function = _lock_function, .unlock_function = _unlock_function, .context = _context}

#define SFHLOG_DECLARE_SINK(_output_function, _shutdown_function, _context) \
	{ .output_function = _output_function, .shutdown_function = _shutdown_function, .context = _context}

#define SFHLOG_DECLARE_CONST_SINK_LIST(...) \
	(struct sfhlog_sink*)(const struct sfhlog_sink []) { \
		__VA_ARGS__ \
		, { 0 } \
	}


#define SFHLOG_DECLARE_CONFIG( \
	_name, \
	_sinks, \
	_root_severity, \
	...) \
struct sfhlog_config _name = \
{ \
	.sinks = _sinks, \
	.root = (struct sfhlog_config_node []) { \
        { \
            .path = "", \
            .severity = _root_severity, \
            .children = (struct sfhlog_config_node []) { \
                __VA_ARGS__, \
                {0} \
            } \
        } \
	} \
}

#define SFHLOG_DECLARE_CONST_CONFIG( \
	_name, \
	_sinks, \
	_lock, \
	_root_severity, \
	...) \
const struct sfhlog_config _name = \
{ \
	.sinks = _sinks, \
	.lock = _lock, \
	.root = (struct sfhlog_config_node*)(const struct sfhlog_config_node []) { \
        { \
            .path = "", \
            .severity = _root_severity, \
            .children = (struct sfhlog_config_node*)(const struct sfhlog_config_node []) { \
                __VA_ARGS__, \
                {0} \
            } \
        } \
	} \
}

#define SFHLOG_DECLARE_CONFIG_ENTRY(_path, _severity, ...) \
{ \
	.path = _path, \
	.severity = _severity, \
	.children = (struct sfhlog_config_node []) { \
		__VA_ARGS__, \
		{0} \
	} \
}

#define SFHLOG_DECLARE_CONST_CONFIG_ENTRY(_path, _severity, ...) \
{ \
	.path = _path, \
	.severity = _severity, \
	.children = (struct sfhlog_config_node*)(const struct sfhlog_config_node []) { \
		__VA_ARGS__, \
		{0} \
	} \
}

#define SFHLOG_CONFIG_ENTRY_NOCHILDREN {0}


void sfhlog_init(const struct sfhlog_config* config);
void sfhlog_log(
		const struct sfhlog_logger* logger,
		enum sfhlog_severity severity,
		const char* func,
		unsigned int line,
		const char* fmt, ...);


#define LOG_ERROR(logger, fmt, ...) do { sfhlog_log(logger, LOG_SEVERITY_ERROR, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define LOG_WARN(logger, fmt, ...) do { sfhlog_log(logger, LOG_SEVERITY_WARN, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define LOG_INFO(logger, fmt, ...) do { sfhlog_log(logger, LOG_SEVERITY_INFO, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)
#define LOG_DEBUG(logger, fmt, ...) do { sfhlog_log(logger, LOG_SEVERITY_DEBUG, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); } while(0)


__END_DECLS;

#endif
