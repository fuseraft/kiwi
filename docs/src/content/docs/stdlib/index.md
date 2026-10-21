---
title: Standard Library
description: Every package in Kiwi's standard library, grouped by purpose.
sidebar:
  label: Overview
  order: 0
---

The standard library is loaded automatically: every package below is available in every script without an `import`. Call a package's functions with `package::name(...)`.

Each package page starts with a guide and examples, followed by an API reference generated from the docstrings in `lib/*.kiwi`.

## Data formats

| Package | Purpose |
| :-- | :-- |
| [`csv`](/kiwi/stdlib/csv/) | Parse CSV into lists or hashmaps, with typed columns. |
| [`json`](/kiwi/stdlib/json/) | Parse and produce JSON. |
| [`yaml`](/kiwi/stdlib/yaml/) | Parse and produce YAML. |
| [`xml`](/kiwi/stdlib/xml/) | Parse, query, build and serialize XML. |
| [`protobuf`](/kiwi/stdlib/protobuf/) | Encode and decode Protocol Buffers. |
| [`encode`](/kiwi/stdlib/encode/) | Base64, hex and URL encoding. |
| [`compress`](/kiwi/stdlib/compress/) | DEFLATE, GZip, Brotli, Zstandard and ZIP. |
| [`crypto`](/kiwi/stdlib/crypto/) | MD5, SHA and HMAC hashes. |

## Working with data

| Package | Purpose |
| :-- | :-- |
| [`table`](/kiwi/stdlib/table/) | Filter, join, group and display lists of hashmaps. |
| [`text`](/kiwi/stdlib/text/) | Line-oriented text pipelines. |
| [`pipeline`](/kiwi/stdlib/pipeline/) | Named, step-by-step processing pipelines. |
| [`sqlite`](/kiwi/stdlib/sqlite/) | SQLite databases. |
| [`stat`](/kiwi/stdlib/stat/) | Descriptive statistics. |
| [`math`](/kiwi/stdlib/math/) | Mathematical functions and random numbers. |
| [`collections`](/kiwi/stdlib/collections/) | `Heap` and `Set`. |
| [`iter`](/kiwi/stdlib/iter/) | A cursor for stepping through lists. |
| [`faker`](/kiwi/stdlib/faker/) | Realistic fake data. |

## Built-in type extensions

| Package | Purpose |
| :-- | :-- |
| [`string`](/kiwi/stdlib/string/) | String formatting and inspection, as string methods. |
| [`list`](/kiwi/stdlib/list/) | List predicates and searches, as list methods. |
| [`date`](/kiwi/stdlib/date/) | Date arithmetic and components, as date methods. |

## Text and output

| Package | Purpose |
| :-- | :-- |
| [`fmt`](/kiwi/stdlib/fmt/) | `printf`-style formatting. |
| [`template`](/kiwi/stdlib/template/) | `{{placeholder}}` templates. |
| [`regex`](/kiwi/stdlib/regex/) | Regular expressions. |
| [`console`](/kiwi/stdlib/console/) | Terminal colors, cursor and key input. |
| [`cli`](/kiwi/stdlib/cli/) | Messages, tables, prompts and argument parsing for command-line tools. |
| [`log`](/kiwi/stdlib/log/) | Leveled logging. |

## Files and system

| Package | Purpose |
| :-- | :-- |
| [`fio`](/kiwi/stdlib/fio/) | Read, write and manage files and directories. |
| [`path`](/kiwi/stdlib/path/) | The `Path` struct. |
| [`watch`](/kiwi/stdlib/watch/) | Watch files and directories for changes. |
| [`stdin`](/kiwi/stdlib/stdin/) | Read standard input. |
| [`env`](/kiwi/stdlib/env/) | Environment variables and command-line arguments. |
| [`dotenv`](/kiwi/stdlib/dotenv/) | Load `.env` files. |
| [`sys`](/kiwi/stdlib/sys/) | Run programs and manage processes. |
| [`signal`](/kiwi/stdlib/signal/) | Handle OS signals. |
| [`ipc`](/kiwi/stdlib/ipc/) | Named-pipe messaging between processes. |

## Networking

| Package | Purpose |
| :-- | :-- |
| [`http`](/kiwi/stdlib/http/) | HTTP client. |
| [`httpserver`](/kiwi/stdlib/httpserver/) | HTTP server with routing and middleware. |
| [`curl`](/kiwi/stdlib/curl/) | HTTP requests through the system `curl`. |
| [`socket`](/kiwi/stdlib/socket/) | TCP and TLS sockets (`socket` and `tls`). |
| [`udp`](/kiwi/stdlib/udp/) | UDP datagrams. |
| [`smtp`](/kiwi/stdlib/smtp/) | Send email. |

## Concurrency and time

| Package | Purpose |
| :-- | :-- |
| [`task`](/kiwi/stdlib/task/) | Background tasks, channels and mutexes. |
| [`time`](/kiwi/stdlib/time/) | The clock, timestamps and calendar helpers. |
| [`stopwatch`](/kiwi/stdlib/stopwatch/) | Measure elapsed time. |

## Development

| Package | Purpose |
| :-- | :-- |
| [`tester`](/kiwi/stdlib/tester/) | Unit tests. |
| [`bench`](/kiwi/stdlib/bench/) | Benchmarks. |
| [`error`](/kiwi/stdlib/error/) | Structured errors and guards. |
| [`reflector`](/kiwi/stdlib/reflector/) | Runtime introspection. |
| [`hashable`](/kiwi/stdlib/hashable/) | Convert struct instances to hashmaps. |
| [`std`](/kiwi/stdlib/std/) | Small general helpers. |

Global helpers such as `d()`, `char()`, `spawn()` and `await()` are on the [Global Functions](/kiwi/reference/global-functions/) page.
