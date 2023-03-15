Compile-time initialization
===========================

Introduction
------------

Compile time initialization is used on embedded systems without an operating system and storage.
Configurations are stored in ROM (flash) memory as much as possible to minimize impact on RAM.


Defining a filter
------------------

A filter defines log levels for nodes in the logger namespace tree.


.. code-block:: C

    enxlog_filter(my_filter_tree)
        enxlog_filter_entry("main", LOGLEVEL_ERROR)
        enxlog_end_filter_entry()
        enxlog_filter_entry("system", LOGLEVEL_ERROR)
            enxlog_filter_entry("monitor", LOGLEVEL_ERROR)
                enxlog_end_filter_entry()
            enxlog_end_filter_entry()
            enxlog_filter_entry("control", LOGLEVEL_ERROR)
                enxlog_filter_entry("valves", LOGLEVEL_DEBUG)
                enxlog_end_filter_entry()
            enxlog_end_filter_entry()
        enxlog_end_filter_entry()
    enxlog_end_filter()


The filter defined above configures all the nodes to LOGLEVEL_ERROR, except for "system.control.valves" which is set to LOGLEVEL_DEBUG.


Defining a sink list
--------------------

Define a compile-time sink list as follows:

.. code-block:: C

    enxlog_sink_list(my_sink_list)
        enxlog_sink(
            NULL,
            NULL,
            NULL,
            enxlog_sink_stdout_log_entry_open,
            enxlog_sink_stdout_log_entry_write,
            enxlog_sink_stdout_log_entry_close
        )
    enxlog_end_sink_list()


.. c:macro:: enxlog_sink(context, fn_init, fn_shutdown, fn_log_entry_open, fn_log_entry_write, fn_log_entry_close)

   :param context: A context pointer passed to the sink functions
   :param fn_init: The sink initialization function. See :c:func:`enxlog_sink_init_fn_t`
   :param fn_shutdown: The sink shutdown function. See :c:func:`enxlog_sink_shutdown_fn_t`
   :param fn_log_entry_open: The sink log entry open function. See :c:func:`enxlog_sink_log_entry_open_fn_t`
   :param fn_log_entry_open: The sink log entry write function. See :c:func:`enxlog_sink_log_entry_write_fn_t`
   :param fn_log_entry_open: The sink log entry close function. See :c:func:`enxlog_sink_log_entry_close_fn_t`
   

Defining a lock
---------------

On systems running an RTOS or other operating system, multiple threads may log at the same time.
The user must provide a locking mechanism to prevent race conditions.

Define a compile-time lock as follows:

.. code-block:: C

    enxlog_lock(_name, _context, _fn_lock, _fn_unlock)


Initialization function
-----------------------

The logging system is initialized with the bleh function.



