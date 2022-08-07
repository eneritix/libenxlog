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

add_library(sfhlog STATIC
    src/enxlog.c
    src/sinks/enxlog_sink_stdout.c
    src/sinks/enxlog_sink_stdout_color.c
    src/sinks/enxlog_sink_file.c
    src/config/enxlog_config_sink_factory.c
    src/config/enxlog_config_sink_parameters.c
    src/config/enxlog_config_parser.c
    )

target_include_directories(sfhlog PUBLIC include)


add_executable(test_basic test/test_basic.c test/test_utils.c)
target_link_libraries(test_basic sfhlog)

add_executable(test_file_sink test/test_file_sink.c test/test_utils.c)
target_link_libraries(test_file_sink sfhlog)

add_executable(test_config_parser test/test_config_parser.c test/test_utils.c)
target_link_libraries(test_config_parser sfhlog)
