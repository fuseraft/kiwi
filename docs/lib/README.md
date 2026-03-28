# Kiwi Standard Library

The standard library is a collection of packages containing useful functions and structs which Kiwi provides out of the box.

### Package Descriptions

| Package | Description |
| --- | --- |
| [`bench`](bench.md) | A benchmarking framework for measuring and reporting code performance. |
| [`cli`](cli.md) | Output helpers, tables, progress bars, prompts, and argument parsing for CLI apps. |
| [`collections`](collections.md) | Specialized collection types, including `Heap` and `Set`. |
| [`compress`](compress.md) | Functions for compression and decompression. |
| [`console`](console.md) | An interface that wraps core I/O operations. |
| [`crypto`](crypto.md) | Cryptographic functions like MD5 and SHA-2. |
| [`csv`](csv.md) | Functions for parsing CSV data. |
| [`curl`](curl.md) | A thin wrapper around the system `curl` command for making HTTP requests. |
| [`date`](date.md) | Extends the built-in `date` type with arithmetic and component accessor methods. |
| [`dotenv`](dotenv.md) | Load `.env` files into the process environment. |
| [`encode`](encode.md) | Encoding and decoding utilities: Base64, hex, and URL. |
| [`env`](env.md) | For interacting with environment variables. |
| [`error`](error.md) | Helpers for constructing, throwing, and inspecting structured errors. |
| [`faker`](faker.md) | Realistic fake data for testing and development: names, addresses, emails, lorem, and more. |
| [`fio`](fio.md) | Functions for file system operations. |
| [`fmt`](fmt.md) | `sprintf`-style string formatting with `%` specifiers. |
| [`hashable`](hashable.md) | The `Hashable` mixin struct for automatic hashmap serialization of instance variables. |
| [`http`](http.md) | Functions for HTTP requests (GET, POST, etc), including `HttpResponse`. |
| [`httpserver`](httpserver.md) | Production-grade HTTP server with routing, middleware, static files, CORS, cookies, and Tomcat-style configuration. |
| [`ipc`](ipc.md) | Interprocess communication via named pipes with length-prefixed message framing. |
| [`iter`](iter.md) | An explicit iterator pattern for safely traversing lists. |
| [`log`](log.md) | Structured logging with severity levels and optional file output. |
| [`json`](json.md) | Functions for parsing and serializing JSON data. |
| [`list`](list.md) | Higher-order utility functions for working with lists. |
| [`math`](math.md) | Common mathematical functions and utilities. |
| [`path`](path.md) | Contains the definition for the `Path` struct. |
| [`pipeline`](pipeline.md) | Named, step-based data pipelines with before/after hooks and error recovery. |
| [`protobuf`](protobuf.md) | Encode and decode data in Protocol Buffer binary wire format (proto3). |
| [`reflector`](reflector.md) | Reflection tools, including `Callable`. |
| [`regex`](regex.md) | Regular expression matching, searching, splitting, and replacement. |
| [`signal`](signal.md) | OS signal handling for `SIGINT`, `SIGTERM`, and `SIGHUP`. |
| [`smtp`](smtp.md) | Send email via SMTP with a fluent `Message` builder. |
| [`socket` / `tls`](socket.md) | Low-level asynchronous TCP and TLS networking. |
| [`sqlite`](sqlite.md) | SQLite database access via the `SqliteDb` struct. |
| [`stat`](stat.md) | Descriptive statistics: mean, median, variance, correlation, and more. |
| [`std`](std.md) | General-purpose utility functions for introspection and common operations. |
| [`stdin`](stdin.md) | Functions for reading from standard input. |
| [`stopwatch`](stopwatch.md) | A simple elapsed-time stopwatch with lap support. |
| [`string`](string.md) | String manipulation and transformation utilities. |
| [`sys`](sys.md) | For executing shell commands. |
| [`table`](table.md) | In-memory tabular data pipeline: filter, join, group, aggregate, and display lists of hashmaps. |
| [`task`](task.md) | Asynchronous task management, including `Channel`. |
| [`template`](template.md) | Lightweight `{{key}}` string template engine. |
| [`tester`](tester.md) | A simple unit testing framework. |
| [`text`](text.md) | Fluent line-oriented text processing pipeline with filter, map, match, and split steps. |
| [`time`](time.md) | Time and date utilities. |
| [`udp`](udp.md) | UDP socket communication for sending and receiving datagrams. |
| [`xml`](xml.md) | XML parser and serializer using a tree of `XmlNode` structs. |
| [`yaml`](yaml.md) | YAML parser and serializer. |
