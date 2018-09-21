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


SFHLOG_DECLARE_CONFIG(
	logger_config,
	SFHLOG_DECLARE_CONST_SINK_LIST(
		SFHLOG_DECLARE_SINK(sfhlog_sink_stdout, 0, 0)
	),
	LOG_SEVERITY_NONE,

	SFHLOG_DECLARE_CONFIG_ENTRY("one", LOG_SEVERITY_ERROR,
		SFHLOG_DECLARE_CONFIG_ENTRY("two", LOG_SEVERITY_WARN,
			SFHLOG_DECLARE_CONFIG_ENTRY("three", LOG_SEVERITY_INFO,
				SFHLOG_CONFIG_ENTRY_NOCHILDREN
			)
		)
	)
);


LOGGER_DECLARE(logger_one, "one");
LOGGER_DECLARE(logger_two, "one", "two");
LOGGER_DECLARE(logger_three, "one", "two", "three");


int main(void)
{
	sfhlog_init(&logger_config);

	LOG_ERROR(logger_one, "This should print");
	LOG_WARN(logger_two, "This should print");
	LOG_INFO(logger_three, "This should print");

	return 0;
}
