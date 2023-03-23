API Reference
=============


Filter definition macros
------------------------
.. doxygendefine:: enxlog_filter

.. doxygendefine:: enxlog_filter_entry

.. doxygendefine:: enxlog_end_filter



Sink definition macros
----------------------

.. doxygendefine:: enxlog_sink_list

.. doxygendefine:: enxlog_sink

.. doxygendefine:: enxlog_end_sink_list


Sink callback functions
-----------------------

.. doxygentypedef:: enxlog_sink_init_fn_t

.. doxygentypedef:: enxlog_sink_shutdown_fn_t

.. doxygentypedef:: enxlog_sink_log_entry_open_fn_t

.. doxygentypedef:: enxlog_sink_log_entry_write_fn_t

.. doxygentypedef:: enxlog_sink_log_entry_close_fn_t


Lock definition macros
----------------------

.. doxygendefine:: enxlog_lock


Lock callback functions
-----------------------

.. doxygentypedef:: enxlog_lock_lock_fn_t

.. doxygentypedef:: enxlog_lock_unlock_fn_t


Initialization and Shutdown
---------------------------

.. doxygenfunction:: enxlog_init

.. doxygenfunction:: enxlog_shutdown


Logging Macros
--------------

.. doxygendefine:: LOGGER

.. doxygendefine:: LOG_ERROR

.. doxygendefine:: LOG_WARN

.. doxygendefine:: LOG_INFO

.. doxygendefine:: LOG_DEBUG

