# The Kiwi Standard Library

The Kiwi Standard Library is a set of modules that enable easier systems programming in Kiwi.

| **Module** | **Description** |
| --- | --- |
| [`@kiwi/argv`](lib/argv.md) | A module for reading command-line arguments. |
| [`@kiwi/env`](lib/env.md) | A module for reading environment variables. |
| [`@kiwi/fs`](lib/fs.md) | A module for working with the filesystem. |
| [`@kiwi/math`](lib/math.md) | A module of useful math functions. |
| [`@kiwi/sys`](lib/sys.md) | A module for working with the OS shell. |
| [`@kiwi/time`](lib/time.md) | A module with useful date and time functions. |

# Experimental Modules

Kiwi supports some experimental modules for working with databases and making HTTP requests.

| **Module** | **Description** |
| --- | --- |
| [`@kiwi/http`](lib/http.md) | A module for making HTTP requests. |
| [`@kiwi/odbc`](lib/odbc.md) | A module for working with common databases. |


Note: To use these modules, you need to install the following dependencies.

#### Fedora / RHEL

```bash
sudo dnf update
sudo dnf install libcurl-devel  # dependency for @kiwi/http
sudo dnf install unixODBC-devel # dependency for @kiwi/odbc
```

#### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install unixodbc-dev
```

#### Build with Experimental Modules
To build Kiwi with experimental modules enabled.

```bash
make experimental
```