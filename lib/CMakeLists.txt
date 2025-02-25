###############################################################################
#
#  Copyright (c) 2018 Eneritix (Pty) Ltd
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#
###############################################################################

option(LIBENXLOG_CONFIG_PARSER "Include runtime configuration parser" ON)

set(enxlog_SOURCES
    source/enxlog.c
    source/sinks/enxlog_sink_stdout.c
    source/sinks/enxlog_sink_stdout_color.c
    source/sinks/enxlog_sink_file.c
    )

if (LIBENXLOG_CONFIG_PARSER)
    set(enxlog_SOURCES
        ${enxlog_SOURCES}
        source/config/enxlog_config_parser.c
        source/config/enxlog_filter_config.c
        source/config/enxlog_sink_config.c
        source/config/enxlog_sink_factory.c
        source/config/enxlog_sink_parameters.c
        )
endif(LIBENXLOG_CONFIG_PARSER)

add_library(enxlog STATIC
    ${enxlog_SOURCES}
)

target_compile_options(enxlog PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter)

target_include_directories(enxlog PUBLIC include)

if (LIBENXLOG_CONFIG_PARSER)
    target_link_libraries(enxlog PUBLIC enxtxt yaml)
else()
    target_link_libraries(enxlog PUBLIC enxtxt)
endif(LIBENXLOG_CONFIG_PARSER)

