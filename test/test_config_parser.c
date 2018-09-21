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
#include <sfhlog/config/sfhlog_config_parser.h>

#include <stdio.h>


LOGGER_DECLARE(a, "a");
LOGGER_DECLARE(b, "a", "b");
LOGGER_DECLARE(c, "a", "b", "c");

LOGGER_DECLARE(one, "one");
LOGGER_DECLARE(two, "one", "two");
LOGGER_DECLARE(three, "one", "two", "three");



void error_callback(int line, int column, const char* message)
{
	printf("%d:%d: %s\n", line, column, message);
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("usage: test_config_file <logger_config>\n");
		return 1;
	}

	struct sfhlog_config* config = sfhlog_config_parse(argv[1], NULL, error_callback);
	if (config == NULL) {
		printf("Error parsing config file!\n");
		return -1;
	}

	sfhlog_init(config);


	LOG_DEBUG(a, "This should not display");
	LOG_DEBUG(b, "This should not display");
	LOG_DEBUG(c, "This should display");

	LOG_DEBUG(one, "This should not display");
	LOG_DEBUG(two, "This should not display");
	LOG_DEBUG(three, "This should display");

	sfhlog_config_destroy(config);

	return 0;
}
