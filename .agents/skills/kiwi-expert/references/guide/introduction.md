# Introduction

> What Kiwi is, what it is good at, and how these docs are organized.

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

- **Scripts and command-line tools.** Argument parsing, colored output, tables, prompts and progress bars are in the [`cli`](../lib/cli.md) package; shell commands are one call away in [`sys`](../lib/sys.md).
- **Working with data.** Read and write [CSV](../lib/csv.md), [JSON](../lib/json.md), [YAML](../lib/yaml.md) and [XML](../lib/xml.md), query [SQLite](../lib/sqlite.md), and reshape rows with [`table`](../lib/table.md).
- **Small services.** An [HTTP server](../lib/httpserver.md) with routing and middleware, an [HTTP client](../lib/http.md), [TCP/TLS](../lib/socket.md) and [UDP](../lib/udp.md) sockets, and [background tasks](../lib/task.md) with channels.
- **Learning and prototyping.** A [REPL](../tools/repl.md) with multi-line editing and a step [debugger](../tools/debugger.md).

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

- **[Getting Started](installation.md)** gets Kiwi installed and walks through the language in one sitting with [the tour](tour.md).
- **[Language](../language/syntax.md)** covers each part of the language in depth: syntax, types, functions, structs, packages, errors and so on.
- **[Tools](../tools/cli.md)** documents the `kiwi` command, the REPL, the debugger and the test framework.
- **[Reference](../reference/keywords.md)** lists every keyword, every built-in method by type, and the global functions.
- **[Standard Library](../lib/README.md)** has a page per package. The API section of each page is generated from the docstrings in `lib/*.kiwi`, so it always matches the source.

Every Kiwi example in these docs is run against the interpreter in CI, so what you read is what Kiwi does.
