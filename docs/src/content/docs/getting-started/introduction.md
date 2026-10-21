---
title: Introduction
description: What Kiwi is, what it is good at, and how these docs are organized.
---

Kiwi is a dynamically typed scripting language with a small, readable syntax and a large standard library. Blocks are delimited with `end` instead of braces or indentation, strings interpolate with `${...}`, and everything from HTTP to SQLite to CSV ships in the box.

```kiwi
fn greet(name = "world")
  println "Hello, ${name}!"
end

greet()          # Hello, world!
greet("Kiwi")    # Hello, Kiwi!
```

The interpreter is written in C# (.NET 10). Scripts are compiled to bytecode and run on a stack-based virtual machine, and the standard library is itself mostly written in Kiwi.

## What Kiwi is good at

- **Scripts and command-line tools.** Argument parsing, colored output, tables, prompts and progress bars are in the [`cli`](/kiwi/stdlib/cli/) package; shell commands are one call away in [`sys`](/kiwi/stdlib/sys/).
- **Working with data.** Read and write [CSV](/kiwi/stdlib/csv/), [JSON](/kiwi/stdlib/json/), [YAML](/kiwi/stdlib/yaml/) and [XML](/kiwi/stdlib/xml/), query [SQLite](/kiwi/stdlib/sqlite/), and reshape rows with [`table`](/kiwi/stdlib/table/).
- **Small services.** An [HTTP server](/kiwi/stdlib/httpserver/) with routing and middleware, an [HTTP client](/kiwi/stdlib/http/), [TCP/TLS](/kiwi/stdlib/socket/) and [UDP](/kiwi/stdlib/udp/) sockets, and [background tasks](/kiwi/stdlib/task/) with channels.
- **Learning and prototyping.** A [REPL](/kiwi/tools/repl/) with multi-line editing and a step [debugger](/kiwi/tools/debugger/).

## A first program

Save this as `hello.kiwi`:

```kiwi
println "Hello, World!"
```

Run it:

```sh
kiwi hello.kiwi
```

There is no `main` function to declare and nothing to import. Top-level statements run from top to bottom, and the whole standard library is available by default.

## How these docs are organized

- **[Getting Started](/kiwi/getting-started/installation/)** gets Kiwi installed and walks through the language in one sitting with [the tour](/kiwi/getting-started/tour/).
- **[Language](/kiwi/language/syntax/)** covers each part of the language in depth: syntax, types, functions, structs, packages, errors and so on.
- **[Tools](/kiwi/tools/cli/)** documents the `kiwi` command, the REPL, the debugger and the test framework.
- **[Reference](/kiwi/reference/keywords/)** lists every keyword, every built-in method by type, and the global functions.
- **[Standard Library](/kiwi/stdlib/)** has a page per package. The API section of each page is generated from the docstrings in `lib/*.kiwi`, so it always matches the source.

Every Kiwi example in these docs is run against the interpreter in CI, so what you read is what Kiwi does.
