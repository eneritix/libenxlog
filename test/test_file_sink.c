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

#include <sfhlog/sfhlog.h>
#include <sfhlog/sinks/sfhlog_sink_stdout.h>
#include <sfhlog/sinks/sfhlog_sink_stdout_color.h>
#include <sfhlog/sinks/sfhlog_sink_file.h>


static struct sfhlog_sink_file_context sink_file_context;

SFHLOG_DECLARE_CONST_CONFIG(
	logger_config,
	SFHLOG_DECLARE_CONST_SINK_LIST(
		SFHLOG_DECLARE_SINK(sfhlog_sink_stdout_color, 0, 0),
		SFHLOG_DECLARE_SINK(sfhlog_sink_stdout, 0, 0),
		SFHLOG_DECLARE_SINK(sfhlog_sink_file, 0, &sink_file_context)
	),
	LOG_SEVERITY_DEBUG,
	SFHLOG_DECLARE_CONST_CONFIG_ENTRY("path", LOG_SEVERITY_WARN, SFHLOG_CONFIG_ENTRY_NOCHILDREN)
);


LOGGER_DECLARE(logger, "test");


int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("usage: test_file_sink <logger_config>\n");
		return 1;
	}

	if (sfhlog_sink_file_init(&sink_file_context, argv[1]) == -1) {
		printf("Could not open output file\n");
		return 1;
	}

	sfhlog_init(&logger_config);

	LOG_ERROR(logger, "This is an error");
	LOG_WARN(logger, "This is a warning");
	LOG_INFO(logger, "This is info");
	LOG_DEBUG(logger, "This is debug data");

	return 0;
}
