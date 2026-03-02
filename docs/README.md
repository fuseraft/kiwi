# Kiwi 🥝

Kiwi is a modern, lightweight scripting language built for expressiveness and utility.

## **Getting Started**

- **[Introduction to Kiwi](introduction.md)**
    - [Building Kiwi](building_kiwi.md)
    - [Using the Kiwi CLI](cli.md)
    - [REPL](repl.md)
    - [Running Scripts & Execution Modes](runners.md)
    - [kdb — Kiwi Debugger](kdb.md)

## **Language Reference**

- [Variables](variables.md)
- [Types](types.md)
- [Type Hints](type_hints.md)
- [Operators](operators.md)
- [Keywords](keywords.md)
- [Comments](comments.md)
- [Docstrings](docstrings.md)

## **Functions**

- [Functions](functions.md)
- [Lambdas](lambdas.md)

## **Control Flow**

- [Control Structures](control_structures.md) — `if`, `case`, `when`, ternary, truthiness
- [Loops](loops.md) — `for`, `while`, `repeat`, `break`, `next`
- [Generators](generators.md)
- [Error Handling](error_handling.md)

## **Collections & Data Types**

- [Lists](lists.md)
- [Ranges](ranges.md)
- [Hashmaps](hashmaps.md)
- [Strings](strings.md)
- [Dates](dates.md)

## **Object-Oriented Programming**

- [Structs](structs.md)
- [Abstract Structs](abstract_structs.md)
- [Operator Overloading](operator_overloading.md)

## **Modules & I/O**

- [Packages](packages.md)
- [Events](events.md)
- [Builtins](builtins.md)
- [Console I/O](console_io.md)

## **Standard Library**

- [`collections`](lib/collections.md) — Specialized collection types, including `Heap` and `Set`.
- [`compress`](lib/compress.md) — Functions for compression and decompression.
- [`console`](lib/console.md) — An interface that wraps core I/O operations.
- [`crypto`](lib/crypto.md) — Cryptographic functions like MD5 and SHA-2.
- [`csv`](lib/csv.md) — Functions for parsing CSV data.
- [`env`](lib/env.md) — For interacting with environment variables.
- [`fio`](lib/fio.md) — Functions for file system operations.
- [`http`](lib/http.md) — Functions for HTTP/HTTPS requests.
- [`iter`](lib/iter.md) — An explicit iterator pattern for safely traversing lists.
- [`math`](lib/math.md) — Common mathematical functions and utilities.
- [`path`](lib/path.md) — Contains the definition for the `Path` struct.
- [`reflector`](lib/reflector.md) — Reflection tools, including `Callable`.
- [`socket` / `tls`](lib/socket.md) — Low-level asynchronous TCP and TLS networking.
- [`stdin`](lib/stdin.md) — Functions for reading from standard input.
- [`string`](lib/string.md) — String manipulation and transformation utilities.
- [`sys`](lib/sys.md) — For executing shell commands.
- [`task`](lib/task.md) — Asynchronous task management, including `Channel`.
- [`tester`](lib/tester.md) — A simple unit testing framework.
- [`time`](lib/time.md) — Time and date utilities.
