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
| [`env`](env.md) | For interacting with environment variables. |
| [`error`](error.md) | Helpers for constructing, throwing, and inspecting structured errors. |
| [`faker`](faker.md) | Realistic fake data for testing and development: names, addresses, emails, lorem, and more. |
| [`fio`](fio.md) | Functions for file system operations. |
| [`hashable`](hashable.md) | The `Hashable` mixin struct for automatic hashmap serialization of instance variables. |
| [`http`](http.md) | Functions for HTTP requests (GET, POST, etc), including `HttpResponse`. |
| [`iter`](iter.md) | An explicit iterator pattern for safely traversing lists. |
| [`json`](json.md) | Functions for parsing and serializing JSON data. |
| [`list`](list.md) | Higher-order utility functions for working with lists. |
| [`math`](math.md) | Common mathematical functions and utilities. |
| [`path`](path.md) | Contains the definition for the `Path` struct. |
| [`pipeline`](pipeline.md) | Named, step-based data pipelines with before/after hooks and error recovery. |
| [`protobuf`](protobuf.md) | Encode and decode data in Protocol Buffer binary wire format (proto3). |
| [`reflector`](reflector.md) | Reflection tools, including `Callable`. |
| [`regex`](regex.md) | Regular expression matching, searching, splitting, and replacement. |
| [`socket` / `tls`](socket.md) | Low-level asynchronous TCP and TLS networking. |
| [`sqlite`](sqlite.md) | SQLite database access via the `SqliteDb` struct. |
| [`std`](std.md) | General-purpose utility functions for introspection and common operations. |
| [`stdin`](stdin.md) | Functions for reading from standard input. |
| [`string`](string.md) | String manipulation and transformation utilities. |
| [`sys`](sys.md) | For executing shell commands. |
| [`table`](table.md) | In-memory tabular data pipeline: filter, join, group, aggregate, and display lists of hashmaps. |
| [`task`](task.md) | Asynchronous task management, including `Channel`. |
| [`tester`](tester.md) | A simple unit testing framework. |
| [`text`](text.md) | Fluent line-oriented text processing pipeline with filter, map, match, and split steps. |
| [`time`](time.md) | Time and date utilities. |
| [`xml`](xml.md) | XML parser and serializer using a tree of `XmlNode` structs. |
