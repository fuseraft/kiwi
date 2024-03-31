# The Astral Standard Library

The Astral Standard Library is a set of modules that enable easier systems programming in Astral.

| **Module** | **Description** |
| :--- | :--- |
| [`@astral/argv`](lib/argv.md) | A module for reading command-line arguments. |
| [`@astral/conf`](lib/conf.md) | A module for reading configuration files. |
| [`@astral/env`](lib/env.md) | A module for reading environment variables. |
| [`@astral/fs`](lib/fs.md) | A module for working with the filesystem. |
| [`@astral/math`](lib/math.md) | A module of useful math functions. |
| [`@astral/sys`](lib/sys.md) | A module for working with the OS shell. |
| [`@astral/time`](lib/time.md) | A module with useful date and time functions. |
| [`@astral/http`](lib/http.md) | A module for making HTTP requests. |
| [`@astral/web`](lib/web.md) | A module for building web applications. |

# Experimental Modules

Astral supports some experimental modules for working with databases and making HTTP requests.

| **Module** | **Description** |
| :--- | :--- |
| [`@astral/odbc`](lib/experimental/odbc.md) | A module for working with common databases. |

Note: To use these modules, you need to install the following dependencies.

#### Fedora / RHEL

```bash
sudo dnf update
sudo dnf install unixODBC-devel # dependency for @astral/odbc
```

#### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install unixodbc-dev
```

#### Build with Experimental Modules
To build Astral with experimental modules enabled.

```bash
make experimental
```