Logging
=======

Defining a logger
-----------------

Loggers are defined with the c:macro:`LOGGER()` macro

For example, a logger with variable name 'logger' and logger name 'sys.drivers.uart' will be defined as

.. code-block:: C

    LOGGER(logger, "sys", "drivers", "uart");


Log entries
-----------

A log entry is made by calling one of the :c:macro:`LOG_ERROR`, :c:macro:`LOG_WARN`, :c:macro:`LOG_INFO` or :c:macro:`LOG_DEBUG` macros.


Example
-------

The following example shows how to create a logger and make an entry:

.. code-block:: C

    LOGGER(logger, "sys", "drivers", "uart");

    void my_function(int index, const char *text)
    {
        LOG_DEBUG(logger, "my_function, index={}, text={}", f_int(index), f_str(text));
    }
