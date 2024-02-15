# The Kiwi Common Library: kiwilib

The Kiwi Common Library (**kiwilib**) is a set of modules that enable easier systems programming in Kiwi.

## `@kiwi/argv`

This module contains functionality for working with command-line arguments. 

To learn more, please see [`argv`](kiwilib/argv.md).

## `@kiwi/env`
This module contains functionality for working with environment variables.

To learn more, please see [`env`](kiwilib/env.md).

## `@kiwi/fileio`

This module contains functionality for working with files and directories.

To learn more, please see [`fileio`](kiwilib/fileio.md).

## `@kiwi/math`

This module contains functionality for working with common math operations.

To learn more, please see [`math`](kiwilib/math.md).

## `@kiwi/sys`

This module contains functionality for working with external processes.

To learn more, please see [`sys`](kiwilib/sys.md).

## `@kiwi/time`

This module contains functionality for working with time and date information.

To learn more, please see [`time`](kiwilib/time.md).

# Experimental Modules

Kiwi supports some experimental modules for working with databases and making HTTP requests.

To use these modules, you need to install the following dependencies.

```bash
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install unixodbc-dev
```

To build Kiwi with experimental features enabled.

```bash
make experimental
```

## `@kiwi/http`

This module contains functionality for working with HTTP requests.

To learn more, please see [`http`](kiwilib/http.md).

## `@kiwi/odbc`

This module contains functionality for connecting to databases with ODBC to execute queries, statements, and stored procedures, and use their result sets.

To learn more, please see [`odbc`](kiwilib/odbc.md).