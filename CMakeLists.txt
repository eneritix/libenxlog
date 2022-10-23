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

cmake_minimum_required(VERSION 3.0)
project(libenxlog)

add_subdirectory(../libenxtxt libenxtxt EXCLUDE_FROM_ALL)

option(LIBENXLOG_CONFIG_PARSER "Include runtime configuration parser" ON)

set(enxlog_SOURCES
    src/enxlog.c
    src/sinks/enxlog_sink_stdout.c
    src/sinks/enxlog_sink_stdout_color.c
    src/sinks/enxlog_sink_file.c
    src/sinks/enxlog_sink_factory.c
    src/sinks/enxlog_sink_parameters.c
    )

if (LIBENXLOG_CONFIG_PARSER)
    set(enxlog_SOURCES
        ${enxlog_SOURCES}
        src/config/enxlog_config_parser.c
        src/config/enxlog_filter_config.c
        src/config/enxlog_sink_config.c
        )
endif(LIBENXLOG_CONFIG_PARSER)

add_library(enxlog STATIC
    ${enxlog_SOURCES}
)

target_include_directories(enxlog PUBLIC include)

if (LIBENXLOG_CONFIG_PARSER)
    target_link_libraries(enxlog PUBLIC enxtxt yaml)
endif(LIBENXLOG_CONFIG_PARSER)

add_executable(test_basic test/test_basic.c test/test_utils.c)
target_link_libraries(test_basic enxlog)

add_executable(test_file_sink test/test_file_sink.c test/test_utils.c)
target_link_libraries(test_file_sink enxlog)

add_executable(test_noinit test/test_noinit.c test/test_utils.c)
target_link_libraries(test_noinit enxlog)

if (LIBENXLOG_CONFIG_PARSER)
    add_executable(test_config_parser test/test_config_parser.c test/test_utils.c)
    target_link_libraries(test_config_parser enxlog)
endif(LIBENXLOG_CONFIG_PARSER)
