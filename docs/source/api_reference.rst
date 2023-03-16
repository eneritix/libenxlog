API Reference
=============


Filter declaration macros
-------------------------
.. doxygendefine:: enxlog_filter


Sink declaration macros
-----------------------

Sink callback functions
-----------------------

.. c:function:: bool enxlog_sink_init_fn_t(void *context)

   :param context: Context.
   :returns: a result.

.. c:function:: void enxlog_sink_shutdown_fn_t(void *context)

.. c:function:: void enxlog_sink_log_entry_open_fn_t(void* context, const struct enxlog_logger *logger, enum enxlog_loglevel loglevel, const char *func, unsigned int line)

.. c:function:: void enxlog_sink_log_entry_write_fn_t(void* context, const char *ptr, size_t length)

.. c:function:: void enxlog_sink_log_entry_close_fn_t(void *context)


Lock declaration macros
-----------------------

Lock callback functions
-----------------------

Initialization and Shutdown
---------------------------

.. doxygenfunction:: enxlog_init

