# Standard Library

> Every package in Kiwi's standard library, grouped by purpose.

The standard library is loaded automatically: every package below is available in every script without an `import`. Call a package's functions with `package::name(...)`.

Each package page starts with a guide and examples, followed by an API reference generated from the docstrings in `lib/*.kiwi`.

## Data formats

| Package | Purpose |
| :-- | :-- |
| [`csv`](csv.md) | Parse CSV into lists or hashmaps, with typed columns. |
| [`json`](json.md) | Parse and produce JSON. |
| [`yaml`](yaml.md) | Parse and produce YAML. |
| [`xml`](xml.md) | Parse, query, build and serialize XML. |
| [`protobuf`](protobuf.md) | Encode and decode Protocol Buffers. |
| [`encode`](encode.md) | Base64, hex and URL encoding. |
| [`compress`](compress.md) | DEFLATE, GZip, Brotli, Zstandard and ZIP. |
| [`crypto`](crypto.md) | MD5, SHA and HMAC hashes. |

## Working with data

| Package | Purpose |
| :-- | :-- |
| [`table`](table.md) | Filter, join, group and display lists of hashmaps. |
| [`text`](text.md) | Line-oriented text pipelines. |
| [`pipeline`](pipeline.md) | Named, step-by-step processing pipelines. |
| [`sqlite`](sqlite.md) | SQLite databases. |
| [`stat`](stat.md) | Descriptive statistics. |
| [`math`](math.md) | Mathematical functions and random numbers. |
| [`collections`](collections.md) | `Heap` and `Set`. |
| [`iter`](iter.md) | A cursor for stepping through lists. |
| [`faker`](faker.md) | Realistic fake data. |

## Built-in type extensions

| Package | Purpose |
| :-- | :-- |
| [`string`](string.md) | String formatting and inspection, as string methods. |
| [`list`](list.md) | List predicates and searches, as list methods. |
| [`date`](date.md) | Date arithmetic and components, as date methods. |

## Text and output

| Package | Purpose |
| :-- | :-- |
| [`fmt`](fmt.md) | `printf`-style formatting. |
| [`template`](template.md) | `{{placeholder}}` templates. |
| [`regex`](regex.md) | Regular expressions. |
| [`console`](console.md) | Terminal colors, cursor and key input. |
| [`cli`](cli.md) | Messages, tables, prompts and argument parsing for command-line tools. |
| [`log`](log.md) | Leveled logging. |

## Files and system

| Package | Purpose |
| :-- | :-- |
| [`fio`](fio.md) | Read, write and manage files and directories. |
| [`path`](path.md) | The `Path` struct. |
| [`watch`](watch.md) | Watch files and directories for changes. |
| [`stdin`](stdin.md) | Read standard input. |
| [`env`](env.md) | Environment variables and command-line arguments. |
| [`dotenv`](dotenv.md) | Load `.env` files. |
| [`sys`](sys.md) | Run programs and manage processes. |
| [`signal`](signal.md) | Handle OS signals. |
| [`ipc`](ipc.md) | Named-pipe messaging between processes. |
| [`native`](native.md) | Load native extensions written in C, Rust, Zig or C#. |

## Networking

| Package | Purpose |
| :-- | :-- |
| [`http`](http.md) | HTTP client. |
| [`httpserver`](httpserver.md) | HTTP server with routing and middleware. |
| [`curl`](curl.md) | HTTP requests through the system `curl`. |
| [`socket`](socket.md) | TCP and TLS sockets (`socket` and `tls`). |
| [`udp`](udp.md) | UDP datagrams. |
| [`smtp`](smtp.md) | Send email. |

## Concurrency and time

| Package | Purpose |
| :-- | :-- |
| [`task`](task.md) | Background tasks, channels and mutexes. |
| [`time`](time.md) | The clock, timestamps and calendar helpers. |
| [`stopwatch`](stopwatch.md) | Measure elapsed time. |

## Development

| Package | Purpose |
| :-- | :-- |
| [`tester`](tester.md) | Unit tests. |
| [`bench`](bench.md) | Benchmarks. |
| [`error`](error.md) | Structured errors and guards. |
| [`reflector`](reflector.md) | Runtime introspection. |
| [`hashable`](hashable.md) | Convert struct instances to hashmaps. |
| [`std`](std.md) | Small general helpers. |

Global helpers such as `d()`, `char()`, `spawn()` and `await()` are on the [Global Functions](../reference/global-functions.md) page.
