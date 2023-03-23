Compile-time initialization
===========================

Introduction
------------

Compile time initialization is used on embedded systems without an operating system and storage.
Configurations are stored in ROM (flash) memory as much as possible to minimize impact on RAM.


Defining a filter
------------------

A filter defines log levels for nodes in the logger namespace tree.
Filters are defined with the :c:macro:`enxlog_filter()`, :c:macro:`enxlog_filter_entry()` and :c:macro:`enxlog_end_filter()` macros.


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


Defining a lock
---------------

On systems running an RTOS or other operating system, multiple threads may log at the same time.
The user must provide a locking mechanism to prevent race conditions.

Define a compile-time lock as follows:

.. code-block:: C

    enxlog_lock(_name, _context, _fn_lock, _fn_unlock)


Initialization function
-----------------------

The logging system is initialized with the :c:func:`enxlog_init()` function.


Example
-------

The following example shows how to initialize the library at compile time:

