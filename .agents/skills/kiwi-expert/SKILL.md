---
name: kiwi-expert
description: Expert reference for the Kiwi scripting language — syntax, keywords, operators, types, control flow, structs, error handling, generators, packages, and the full standard library (47 packages). Trigger when writing, reading, reviewing, or debugging .kiwi scripts, or when a question concerns Kiwi language semantics, a stdlib package (fio, http, json, regex, table, task, sqlite, httpserver, etc.), or the VM/runtime's execution rules.
---

# Kiwi Expert

Kiwi is a dynamically-typed scripting language (interpreter in C#/.NET 10, stack-based VM). This skill turns an agent into a precise reference user for the language: a compact cheat sheet for orientation, a full topic-indexed reference set for depth, and a verification habit for anything uncertain.

## When to Use

- Writing, reviewing, or debugging any `.kiwi` script or stdlib module.
- Answering questions about Kiwi syntax, keywords, operators, types, or runtime/scoping semantics.
- Picking the right standard library package or function for a task (file I/O, HTTP, JSON, regex, dates, tabular data, async tasks, etc.).
- Explaining why a piece of Kiwi code behaves a certain way (truthiness, scoping, error shapes, generator laziness).

**Do not use** for the Kiwi *interpreter's* C# internals (that's `src/` — see the repo's `AGENTS.md`), or for unrelated scripting languages.

## Workflow

### Step 1: Orient with the cheat sheet below

The [Core Cheat Sheet](#core-cheat-sheet) covers the syntax you need for almost any task. Most one-off questions are answered by it directly — don't load a reference file unless you need more than it gives you.

### Step 2: Load the specific reference for depth

Use the [Language Reference Index](#language-reference-index) and [Standard Library Index](#standard-library-index) below to pick the *one* file that covers the topic, then:

```
read_skill_resource("kiwi-expert", "references/language/<file>.md")
read_skill_resource("kiwi-expert", "references/lib/<package>.md")
```

Load only what the task needs — these are full docs, not summaries.

### Step 3: Verify anything you're not certain of

The reference files are copied from the project's own docs (`docs/*.md`, `docs/lib/*.md` in the kiwi repo) and are generally reliable, but docs can drift from the interpreter. Before asserting a behavioral claim that a change depends on (not just prose explanation), check it against the real thing rather than trusting prose alone:

- **Fastest**: grep the interpreter source (`src/Parsing/Lexer.cs`, `src/Parsing/Parser.cs`, `src/Parsing/Keyword/`, `lib/*.kiwi`) for the construct in question.
- **Most conclusive**: write a throwaway one-liner and run it — `./bin/kiwi` is a real built binary at the repo root's `bin/kiwi` (rebuild with `./build.sh` if source changed). A script that runs without error is *not* proof a construct does what you think — check its actual output/effect, not just its exit code (see the docstring pitfall below, where invalid-looking syntax silently ran but did nothing).

### Step 4: Apply project conventions

- Functions and struct methods: `snake_case`. Not camelCase.
- Internal/private helpers: prefix with `_`.
- New stdlib module: `lib/mymodule.kiwi` with `package mymodule`, tests in `tests/lib/stdlib/`.
- After any change to `lib/`, `tests/`, or `src/`: rebuild (`./build.sh`) and run `./bin/kiwi tests/test` (exit `0` = pass).

---

## Core Cheat Sheet

### Variables & Types

```kiwi
name = "Kiwi"              # no declaration keyword needed
var count: integer = 0     # optional explicit `var` + type hint
a, b, c = 10, 20, 30       # multiple assignment
```

Built-in types: `integer`, `float`, `boolean`, `string`, `list`, `hashmap`, `null`, `date`, `bytes`, `generator`, `lambda`, `function`, `object` (struct instance), `pointer`.

`typeof(x)` (global function) or `x.type()` (method) both return the type name string.

### Strings

```kiwi
"Hello, ${name}!"    # double-quoted: supports ${} interpolation
'\b[A-Z][a-z]+\b'     # single-quoted: raw, no interpolation (regex/paths)
```

### Control Flow

```kiwi
if cond ... elsif other ... else ... end
status = age >= 18 ? "adult" : "minor"        # ternary

case value
when 1..9:   "single"
when 10, 20: "special"
else:        "other"
end

for item in collection do ... end
for item, idx in list do ... end     # idx starts at 0
for k, v in hashmap do ... end
for x in [1 to 10] do ... end        # inclusive range
while cond do ... end
repeat 5 do ... end
repeat 5 as i do ... end             # i = 1..5

break when guard    # break/next/return/throw all take an optional `when` guard
next when guard
```

Truthy: non-zero numbers, non-empty strings/lists/hashmaps, `true`. Falsy: `0`, `""`, `[]`, `{}`, `false`, `null`.

### Conditionals vs. Loops (critical syntax difference)

**Conditionals (`if` / `case`) — NO `do` keyword:**

```kiwi
if cond ... elsif other ... else ... end
status = age >= 18 ? "adult" : "minor"
case value
when 1..9:   "single"
when 10, 20: "special"
else:        "other"
end
```

**Loops (`for` / `while` / `repeat`) — require `do`:**

```kiwi
for item in collection do ... end
while cond do ... end
repeat 5 do ... end
repeat 5 as i do ... end
```

**Common mistake to avoid:** `if cond do ... end` is invalid.  
Only loops use the `do ... end` form.

### Functions & Lambdas

```kiwi
fn add(a: integer, b: integer): integer
  a + b                    # last expression is the implicit return value
end

add(b = 3, a = 7)          # named arguments, order-independent

square = do (n) => n * n           # lambda, arrow form
double = with (x) do x * 2 end     # lambda, `with` form
[1,2,3].map(do (n) => n * 2)
```

### Structs

```kiwi
struct Animal
  fn new(name)              # constructor
    @name = name            # @field = instance variable
  end
  fn speak() println "${@name} makes a sound." end
  static fn kind() "animal" end
end

struct Dog < Animal          # inheritance
  fn speak()
    super.speak()
    println "${@name} barks!"
  end
end

abstract struct Shape        # cannot be instantiated directly
  abstract fn area() end     # abstract method — subclasses must implement with `override`
end
```

### Error Handling

```kiwi
try
  risky()
catch e                 # e = { "error": "TypeName", "message": "..." }; parens optional
catch t, m               # t = type string, m = message string (destructured form)
finally
  cleanup()
end

throw error::ValueError("bad value")
throw "plain string"     # becomes a generic error
```

### Packages / Imports

```kiwi
math = import "math"     # bound to a local name
math.sqrt(16.0)          # note: sqrt requires a float argument, not an integer
import "http"            # or unbound — most stdlib packages are also directly qualified
http::get(url)           # `pkg::func()` qualified form works once imported
```

### Generators

```kiwi
fn countdown(n)
  while n >= 0 do
    yield n
    n -= 1
  end
end
for x in countdown(5) do ... end   # lazy — pulls one value per iteration
```

### CLI Arguments

Two builtins, two different jobs — **use the right one, they're not interchangeable:**

- `env::args()` — raw, ordered `list` of arguments exactly as passed, duplicates and all. **Use this for `script.kiwi item1 item2 ...` style positional-list programs.**
- `env::argv()` — `hashmap` of key-value args. `-key=value`/`--key=value` → `argv["key"]=="value"`; a bare positional (no `=`) is self-mapped, `argv["5"]=="5"`. Since it's a hashmap, **duplicate positional values silently collapse to one entry** — `kiwi script.kiwi 5 3 8 3 1` → only 4 keys, the repeated `3` is gone. Use `argv()` for actual key-value options, not for a plain arg list.

```kiwi
# kiwi script.kiwi apple banana apple -name=world
env::args()    # ["apple", "banana", "apple", "-name=world"]  (raw, nothing dropped)
env::argv()    # {"apple": "apple", "banana": "banana", "name": "world"}  (deduped, parsed)
```

If you need real flags/options *and* positionals together, use `CliParser` (`import "cli"`) — it gives you a `"_args"` list of positionals separated from parsed flags/options. Caveat: `CliParser`'s `"_args"` is built from `argv()` internally, so it inherits the same duplicate-collapsing limitation — don't rely on it if positionals might repeat. See `references/lib/cli.md`.

### Comments & Docstrings — pitfall

```kiwi
# single-line comment

/#
@summary Adds two integers.
@param integer a : First operand.
@return integer : The sum.
#/
fn add(a, b) a + b end
```

`/# ... #/` is Kiwi's **only** block-comment / docstring syntax (see `references/language/docstrings.md` and `references/language/comments.md`). **There is no `"""..."""` triple-quote docstring construct.** A `"""..."""` block *parses without error* — each `"` sequence is just adjacent string-literal expression statements — but it does nothing: it is not attached to the following declaration and carries no metadata. If you see `"""`-style "docstrings" in older code or notes in this repo, they are a documentation convention error, not real Kiwi syntax — verified live against `./bin/kiwi` (a script using it runs and produces the same output whether the `"""` block is present or removed).

### Common Pitfalls

- No `.length()` — use `.size()` for strings, lists, and hashmaps.
- No `nil` — the only absence value is `null`.
- Loop skip is `next` (not `continue`), with optional `when` guard.
- Identifiers (functions, methods) must be `snake_case`; camelCase is not idiomatic and may not match dispatch conventions in stdlib packages.
- `typeof(x)` is a global function; `.type()` is the equivalent method — both exist, use whichever reads better at the call site.
- Built-in type names (`list`, `hashmap`, `string`, `integer`, `float`, `boolean`, `date`, `any`, `generator`, `lambda`, `object`, `pointer`, `bytes`, `none`) can't be used as a variable, function, or parameter name — `fn foo(list)` is a parse error (`Expected parameter name.`). Use `items`/`data`/`values` etc. instead.
- No `?`-suffixed predicate methods (no `.empty?()`/`.has_key?()` like Ruby) — use `.empty()`/`.has_key()`. A stray trailing `?` is parsed as the ternary operator, not flagged as an unknown method — it produces a confusing cascade of `Expected ':' in ternary operation` / `Expected if-statement or case-statement` errors on unrelated-looking lines. See `references/language/operators.md#ternary--`.
- For a `script.kiwi item1 item2 ...` style positional-arg list, use `env::args()` — **not** `env::argv().keys()`. `argv()` is a hashmap, so it silently drops duplicate positional values; `env::args()` is the raw ordered list and doesn't have that problem. See [CLI Arguments](#cli-arguments) above.

---

## Language Reference Index

`references/language/<file>.md` — load the one that matches the topic.

| Topic | File |
|---|---|
| Gentle intro / walkthrough | `introduction.md` |
| Reserved keywords (full list) | `keywords.md` |
| Built-in types | `types.md` |
| Dot-notation builtins (all types) | `builtins.md` |
| Operators | `operators.md` |
| Operator overloading | `operator_overloading.md` |
| Variables | `variables.md` |
| Type hints | `type_hints.md` |
| Functions | `functions.md` |
| Lambdas | `lambdas.md` |
| Generators (`yield`) | `generators.md` |
| Control structures (overview) | `control_structures.md` |
| Conditionals (`if`/`case`) | `conditionals.md` |
| Loops (`for`/`while`/`repeat`) | `loops.md` |
| Ranges (`[a to b]`, `[a..b]`) | `ranges.md` |
| Strings | `strings.md` |
| Lists | `lists.md` |
| Hashmaps | `hashmaps.md` |
| Structs | `structs.md` |
| Abstract structs | `abstract_structs.md` |
| Enums | `enums.md` |
| Decorators | `decorators.md` |
| Error handling (`try`/`catch`/`throw`) | `error_handling.md` |
| Events (event bus) | `events.md` |
| Scoping rules | `scoping.md` |
| Packages (`package`/`import`/`export`) | `packages.md` |
| Comments | `comments.md` |
| Docstrings (`/# #/` convention) | `docstrings.md` |
| Dates | `dates.md` |
| Console I/O | `console_io.md` |
| REPL | `repl.md` |
| Debugger (`kdb`) | `kdb.md` |
| Script runners / entry points | `runners.md` |
| CLI flags | `cli.md` |
| VM internals (bytecode, stack) | `vm.md` |

## Standard Library Index

`references/lib/<package>.md` — one file per package, imported with `import "name"`.

| Package | Description | File |
|---|---|---|
| `bench` | Benchmarking framework for measuring/reporting performance | `bench.md` |
| `cli` | Output helpers, tables, progress bars, prompts, arg parsing | `cli.md` |
| `collections` | `Heap`, `Set` and other specialized collections | `collections.md` |
| `compress` | Compression/decompression | `compress.md` |
| `console` | Core I/O interface wrapper | `console.md` |
| `crypto` | MD5, SHA-2, HMAC | `crypto.md` |
| `csv` | CSV parsing/writing | `csv.md` |
| `curl` | Wrapper around system `curl` | `curl.md` |
| `date` | `date` type arithmetic and accessors | `date.md` |
| `dotenv` | Load `.env` files into the environment | `dotenv.md` |
| `encode` | Base64, hex, URL encode/decode | `encode.md` |
| `env` | Environment variables | `env.md` |
| `error` | Construct/throw/inspect structured errors | `error.md` |
| `faker` | Fake data: names, addresses, emails, lorem | `faker.md` |
| `fio` | File system operations | `fio.md` |
| `fmt` | `sprintf`-style formatting | `fmt.md` |
| `hashable` | `Hashable` mixin for hashmap serialization | `hashable.md` |
| `http` | HTTP client (GET/POST/etc.), `HttpResponse` | `http.md` |
| `httpserver` | HTTP server: routing, middleware, static files, CORS | `httpserver.md` |
| `ipc` | Named-pipe interprocess communication | `ipc.md` |
| `iter` | Explicit list iterator pattern | `iter.md` |
| `json` | JSON parse/serialize | `json.md` |
| `list` | Higher-order list utilities | `list.md` |
| `log` | Structured logging | `log.md` |
| `math` | Math functions and constants | `math.md` |
| `path` | `Path` struct for filesystem paths | `path.md` |
| `pipeline` | Named, step-based data pipelines | `pipeline.md` |
| `protobuf` | Protocol Buffers (proto3) encode/decode | `protobuf.md` |
| `reflector` | Reflection, `Callable` | `reflector.md` |
| `regex` | Regex match/search/split/replace | `regex.md` |
| `signal` | OS signal handling (`SIGINT`, etc.) | `signal.md` |
| `smtp` | Send email via SMTP | `smtp.md` |
| `socket` / `tls` | Low-level async TCP/TLS networking | `socket.md` |
| `sqlite` | SQLite via `SqliteDb` | `sqlite.md` |
| `stat` | Descriptive statistics | `stat.md` |
| `std` | General introspection/utility functions | `std.md` |
| `stdin` | Read from standard input | `stdin.md` |
| `stopwatch` | Elapsed-time stopwatch | `stopwatch.md` |
| `string` | String manipulation utilities | `string.md` |
| `sys` | Execute shell commands | `sys.md` |
| `table` | In-memory tabular data pipeline | `table.md` |
| `task` | Async tasks, `Channel` | `task.md` |
| `template` | `{{key}}` string templates | `template.md` |
| `tester` | Unit testing framework | `tester.md` |
| `text` | Line-oriented text processing pipeline | `text.md` |
| `time` | Time/date utilities | `time.md` |
| `udp` | UDP socket communication | `udp.md` |
| `xml` | XML parse/serialize (`XmlNode`) | `xml.md` |
| `yaml` | YAML parse/serialize | `yaml.md` |
