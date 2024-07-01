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

set(LIBENXLOG_CONFIG_PARSER CACHE BOOL "Include runtime configuration parser" ON)


add_executable(test_basic source/test_basic.c source/test_utils.c)
target_link_libraries(test_basic enxlog)

add_executable(test_file_sink source/test_file_sink.c source/test_utils.c)
target_link_libraries(test_file_sink enxlog)

add_executable(test_noinit source/test_noinit.c source/test_utils.c)
target_link_libraries(test_noinit enxlog)

add_executable(test_custom_formatter source/test_custom_formatter.c source/test_utils.c)
target_link_libraries(test_custom_formatter enxlog)

add_executable(test_config_parser source/test_config_parser.c source/test_utils.c)
target_link_libraries(test_config_parser enxlog)
