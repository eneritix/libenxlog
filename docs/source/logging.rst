Logging
=======

Declaring a logger
------------------

Loggers are declared with the LOGGER_DECLARE macro

.. c:macro:: LOGGER_DECLARE(name, ...)

   :param name: The variable name of the logger.
   :param ...: A list of strings forming the 'logger name'.

For example, a logger with variable name 'logger' and logger name 'sys.drivers.uart' will be defined as

.. code-block:: C

    LOGGER_DECLARE(logger, "sys", "drivers", "uart");


Log entries
-----------

A log entry is made by calling one of the LOG_ERROR, LOG_WARN, LOG_INFO or LOG_DEBUG macros.


.. c:macro:: LOG_ERROR(logger, format, ...)

   :param logger: The logger object creating the entry.
   :param format: A format string. This is not a printf formatting string but rather a enxtxt format string. See the enxtxt library documentation for more information.
   :param ...: A list enxtxt format function macros. See the enxtxt library documentation for more information.

Example
-------

The following example shows how to create a logger and make an entry:

.. code-block:: C

    LOGGER_DECLARE(logger, "sys", "drivers", "uart");

    void my_function(int index, const char *text)
    {
        LOG_DEBUG(logger, "my_function, index={}, text={}", f_int(index), f_str(text));
    }
