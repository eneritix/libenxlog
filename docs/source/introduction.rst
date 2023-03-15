Introduction
============

libenxlog is a named hierarchical logger library for embedded systems written in C. 
The user can declare named loggers with a name format similar to Java loggers ("com.foo.bar") and then configure visible loglevels for either individual loggers or for a group of loggers.
Loggers are arranged in a hierarchy according to their names. For example, the following three loggers::

    LOGGER_DECLARE(logger1, "project", "main");
    LOGGER_DECLARE(logger2, "project", "system", "monitor");
    LOGGER_DECLARE(logger3, "project", "system", "control", "valves");

will form the following hierarchial tree::

    +-- main
    |
    +-- system
        |
        +-- monitor
        |
        +-- control
            |
            +-- valves
       
In the rest of the documentation this tree will be referred to as the "logger namespace tree".

Configuring the loglevel for "system" will configure the loglevel for all children of "system" unless the loglevel of the children are explicitly configured.
Any logger without an explicit loglevel configuration entry will use the loglevel of the first configured parent, or the default system loglevel if no configured parents are available.


