---
name: kiwi-expert
description: Expert reference for the Kiwi scripting language — syntax, keywords, operators, types, control flow, structs, error handling, generators, packages, and the full standard library (51 packages). Trigger when writing, reading, reviewing, or debugging .kiwi scripts, or when a question concerns Kiwi language semantics, a stdlib package (fio, http, json, regex, table, task, sqlite, httpserver, etc.), or the VM/runtime's execution rules.
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

The [Core Cheat Sheet](#core-cheat-sheet) covers the syntax you need for almost any task, and [Behaviors to Keep in Mind](#behaviors-to-keep-in-mind) lists the places where Kiwi behaves differently from what you might expect. Most one-off questions are answered here directly; don't load a reference file unless you need more.

### Step 2: Load the specific reference for depth

Use the [Reference Index](#reference-index) and [Standard Library Index](#standard-library-index) below to pick the *one* file that covers the topic, then:

```
read_skill_resource("kiwi-expert", "references/language/<file>.md")
read_skill_resource("kiwi-expert", "references/lib/<package>.md")
```

Load only what the task needs: these are full pages, not summaries. Each `references/lib/<package>.md` ends with the package's complete API (every public function and struct method with its signature, parameters and return type).

### Step 3: Verify anything you're not certain of

The reference files are generated from the documentation site (`docs/src/content/docs/` in the kiwi repo) by `cd docs && npm run sync:skill`. Every Kiwi example in those pages is executed against the interpreter in CI, and each package's API section is generated from the docstrings in `lib/*.kiwi`, so they are reliable as of the last sync. Still, before asserting a behavioral claim that a change depends on, check it against the real thing:

- **Fastest**: grep the interpreter source (`src/Parsing/Lexer.cs`, `src/Parsing/Parser.cs`, `src/Runtime/Builtin/Handler/`, `lib/*.kiwi`) for the construct in question.
- **Most conclusive**: write a throwaway script and run it with `./bin/kiwi` (rebuild with `./build.sh` if the source changed). A script that runs without error is *not* proof a construct does what you think: check its actual output, not just its exit code.

### Step 4: Apply project conventions

- Functions and struct methods: `snake_case`. Not camelCase.
- Internal/private helpers: prefix with `_`.
- New stdlib module: `lib/mymodule.kiwi` with `package mymodule`, tests in `tests/lib/stdlib/`, and a docs page `docs/src/content/docs/stdlib/mymodule.md` ending in `<!-- api: mymodule -->`.
- Public stdlib functions need `/# @summary ... @param type name : desc ... @return type : desc #/` docstrings: they *are* the API documentation.
- After any change to `lib/`, `tests/`, or `src/`: rebuild (`./build.sh`) and run `./bin/kiwi tests/test` (exit `0` = pass). After changing documented behavior, update the docs page and run `npm run check:examples` in `docs/`.

---

## Core Cheat Sheet

### Variables & Types

```kiwi
name = "Kiwi"              # no declaration keyword needed
var count: integer = 0     # optional explicit `var` + type hint
const MAX = 3              # constants: UPPER_SNAKE_CASE only
a, b, c = 10, 20, 30       # multiple assignment; x, y = [1, 2] unpacks a list
```

Built-in types: `integer`, `float`, `boolean`, `string`, `list`, `hashmap`, `null`, `date`, `bytes`, `generator`, `lambda`, and struct instances (`typeof` returns the struct name). Named functions and lambdas both report `typeof()` as `lambda`.

`typeof(x)` (global) and `x.type()` (method) both return the type name. Wrap number literals in parentheses to call methods: `(42).to_string()`.

`+` concatenates strings, lists and bytes (`b"ab" + b"cd"`).

Lists and hashmaps are **shared by reference**: `b = a` or `row = grid[0]` aliases the same collection. Use `.clone()` for a deep copy.

### Strings

```kiwi
"Hello, ${name}!\n"   # double-quoted: escapes and ${} interpolation
'\b[A-Z][a-z]+\b'     # single-quoted: raw (backslashes and ${} kept as written)
```

`.replace` and `.split` are literal; `.rreplace`, `.rsplit`, `.scan`, `.find`, `.match`, `.matches` take regexes. Slicing: `s[1:3]`, `s[::-1]`; no negative indexes.

### Control Flow

```kiwi
if cond ... elsif other ... else ... end      # no `do`; `if` is a statement, not an expression
status = age >= 18 ? "adult" : "minor"        # ternary

label = case value                            # case is an expression
  when 1..9   "single digit"                  # inclusive range, no list built
  when 10, 20 "special"                       # several values
  else        "other"
end

for item in collection do ... end
for item, idx in list do ... end     # idx starts at 0
for k, v in hashmap do ... end       # one variable = keys
for x in [1 to 10] do ... end        # inclusive; [10 to 1] counts down
while cond do ... end
repeat 5 as i do ... end             # i = 1..5

break when guard    # break/next/return/throw/exit and assignments take a `when` guard
next when guard     # `next`, not `continue`
exit 1 when failed  # bare `exit` exits with 0
```

Falsy: `false`, `null`, `0`, `0.0`, `""`, `[]`, `{}`. Everything else is truthy, including `"0"` and `[0]`.

An expression continues onto the next line when a binary operator ends the line or starts the next one (except `-`, which starts a new statement). Method chains may start a line with `.`.

### Functions & Lambdas

```kiwi
fn add(a, b = 0)
  a + b                    # last expression is the implicit return value
end

add(b: 3, a: 7)            # named arguments (also `b = 3`)
fn sum(*nums) nums.sum() end
sum(*[1, 2, 3])            # splat

square = do (n) => n * n           # arrow form
double = with (x) do x * 2 end     # `with` form
shout = do (s)                     # block form
  s.uppercase()
end
[1, 2, 3].map(do (n) => n * 2)

fn make_adder(n)
  do (x) => x + n          # returning a lambda; closures capture locals (and `@` in methods)
end
```

Top-level functions are hoisted. A `fn` checks its arguments: a missing required argument, too many positional arguments, or the same parameter by position and by name raise errors. Lambdas are lenient (missing arguments are `null`, extras are ignored) so they work as callbacks. Type hints are enforced at runtime (`TypeError`); a `float` hint also accepts integers.

### Structs

```kiwi
struct Animal
  fn new(name)               # constructor: Animal.new("Rex")
    @name = name             # instance variable; also readable/writable as obj.name
  end
  fn speak() "..." end
  static fn kind() "animal" end
  fn to_string() "Animal(${@name})" end   # used by println and "${obj}"
  private fn secret() 42 end  # callable only from the struct (and children)
end

struct Dog < Animal          # single inheritance
  fn speak()
    super.speak() + " Woof"  # super.method() calls the parent version
  end
end

abstract struct Shape        # cannot be instantiated
  abstract fn area() end     # children must define area(); `override` is optional
end
```

An instance whose struct defines `call` is callable: `obj(x)` is `obj.call(x)`, so lambdas and objects are interchangeable wherever something calls a value.

```kiwi
struct Counter
  static @count = 0          # static variable: @@count inside, Counter.count outside
  fn new() @@count += 1 end  # child structs share the parent's static variables
end
```

Operators `+ - * / // % ** == != < <= > >= & | ^ << >> >>>` can be overloaded with `fn +(other) ... end`. Decorators work on methods too.

### Error Handling

```kiwi
try
  risky()
catch e                  # e = {"error": "TypeName", "message": "..."}; parens optional
  println e["message"]
finally
  cleanup()              # always runs, including after return/break/next
end                      # without a catch, the error propagates after finally

try
  risky()
catch kind, message      # destructured form
end

throw error::ValueError("bad value")
throw "plain string"     # becomes {"error": "KiwiError", "message": "plain string"}
```

### Packages & Files

```kiwi
package geometry
  const PI = 3.14159            # package-scoped: geometry::PI outside
  fn area(r) PI * r * r end     # siblings call each other unqualified
end
geometry::area(2.0)             # usable as soon as declared; `export` is not needed

import "lib/helpers"            # load a file (relative to this file, .kiwi optional), once
m = import "math"               # package reference: m.sqrt(16.0)
```

The whole standard library is preloaded; call it directly (`json::parse(...)`, `fio::read(...)`). A package named after a built-in type (`string`, `list`, `date`, ...) adds every function as a method on that type.

### Generators

```kiwi
fn countdown(n)
  while n >= 0 do
    yield n
    n -= 1
  end
end
for x in countdown(5) do ... end   # lazy; a generator is exhausted after one pass
```

Struct methods and lambdas can `yield` too.

### CLI Arguments

- `env::args()` — raw, ordered `list` of arguments, duplicates kept. **Use this for positional lists.**
- `env::argv()` — `hashmap`: `-key=value` → `"key": "value"`, `--flag` → `"flag": "true"`, a positional maps to itself. Duplicate positionals collapse.
- `cli::CliParser` — flags and typed options plus an ordered `"_args"` list of positionals. See `references/lib/cli.md`.

### Comments & Docstrings

```kiwi
# single-line comment
/# block comment / docstring #/

/#
@summary Adds two integers.
@param integer a : First operand.
@return integer : The sum.
#/
fn add(a, b) a + b end
```

`/# ... #/` is the only block comment and docstring syntax. There is no `"""..."""` docstring: such a block parses as adjacent string expressions and does nothing.

### Common Pitfalls

- Type names (`list`, `hashmap`, `string`, `integer`, `float`, `boolean`, `date`, `any`, `none`, `generator`, `lambda`, `object`, `pointer`, `bytes`) can't be used as variable, parameter or function names, bare hashmap keys, or `.member` names. Use `items`/`data`, and quote keys: `{"list": 1}`, `h["bytes"]`.
- No `?`-suffixed predicates (`.empty?()`); a stray `?` parses as a ternary and produces confusing errors on later lines.
- No `nil`; the absence value is `null`. `x ?? default` only falls back on `null`.
- `/` always returns a float; use `//` for floor division. `**` is left-associative at the same precedence as `*`. Comparisons bind tighter than `&`, `|` and `^`.
- Reading a variable that was never assigned raises `VariableUndefinedError` (it is not `null`). `var name` without a value declares it as `null`.
- Inside a function, assigning to a name that exists as a global updates the global; use `var name = ...` to declare a local instead.

### Behaviors to Keep in Mind

- **Blocks don't create scope**: variables assigned in `if`/`for`/`while` stay visible afterwards, and loop variables keep their last value. Each `for` iteration still gives closures created in it their own copy of the loop variable.
- Instance variables are readable and writable from outside (`obj.name = 1`); use methods for anything with rules.
- `.rreplace` and `.rsplit` are regex replace-all and regex split (the `r` is regex, not right-to-left).
- `.map()` and `.filter()` pass only the element to the lambda; `.each()` also passes the index.
- `tls::tcpconnect` and `tls::accept` return a task id; `task::await` it to get the socket.
- `task::interval(ms, fn)` returns an `Interval`; call `.stop()` (or return `false` from `fn`), or the program won't exit.

---

## Reference Index

Files under `references/`, one per documentation page.

| Topic | File |
|---|---|
| What Kiwi is | `guide/introduction.md` |
| Install / build from source / Docker | `guide/installation.md` |
| The whole language on one page | `guide/tour.md` |
| Statements, comments, identifiers, literals, reserved words, docstrings | `language/syntax.md` |
| Variables, `var`, `const`, unpacking | `language/variables.md` |
| Types, truthiness, equality, conversions, bytes, dates | `language/types.md` |
| Operators and precedence | `language/operators.md` |
| Strings, escapes, interpolation, slicing, regex methods | `language/strings.md` |
| Lists and ranges | `language/lists.md` |
| Hashmaps | `language/hashmaps.md` |
| `if`, `case`, `when` guards, ternary, `exit` | `language/control-flow.md` |
| `for`, `while`, `repeat`, `break`, `next` | `language/loops.md` |
| Functions, defaults, named/variadic args, hoisting | `language/functions.md` |
| Lambdas and closures | `language/lambdas.md` |
| Scoping rules | `language/scoping.md` |
| Type hints | `language/type-hints.md` |
| Errors (`try`/`catch`/`finally`/`throw`) | `language/errors.md` |
| Structs, inheritance, `super`, static, abstract | `language/structs.md` |
| Operator overloading | `language/operator-overloading.md` |
| Enums | `language/enums.md` |
| Packages, `import`/`include`/`require`, type extension | `language/packages.md` |
| Generators (`yield`) | `language/generators.md` |
| Decorators | `language/decorators.md` |
| Events (`on`/`once`/`emit`/`off`) | `language/events.md` |
| `kiwi` command, flags, args, safe mode | `tools/cli.md` |
| REPL | `tools/repl.md` |
| Debugger (`kdb`) | `tools/debugger.md` |
| Writing tests with `tester` | `tools/testing.md` |
| All keywords | `reference/keywords.md` |
| Every built-in method, by type | `reference/builtin-methods.md` |
| Global functions (`println`, `input`, `typeof`, `serialize`, `d()`, ...) | `reference/global-functions.md` |
| Bytecode VM and opcodes | `internals/vm.md` |

## Standard Library Index

`references/lib/<package>.md`: a guide followed by the full generated API. `references/lib/README.md` groups the packages by purpose. Everything is preloaded; no `import` needed.

| Package | Description | File |
|---|---|---|
| `bench` | Benchmarks with timing statistics | `bench.md` |
| `cli` | Labeled output, tables, progress bars, prompts, `CliParser` | `cli.md` |
| `collections` | `Heap` (priority queue) and `Set` | `collections.md` |
| `compress` | DEFLATE, GZip, Brotli, Zstandard, ZIP | `compress.md` |
| `console` | Terminal colors, cursor, key input | `console.md` |
| `crypto` | MD5, SHA-1/2, HMAC, secure random bytes, UUIDs | `crypto.md` |
| `csv` | Parse CSV, typed pipelines | `csv.md` |
| `curl` | HTTP via the system `curl` | `curl.md` |
| `date` | Date arithmetic and components as date methods | `date.md` |
| `dotenv` | Load `.env` files | `dotenv.md` |
| `encode` | Base64, hex, URL encoding | `encode.md` |
| `env` | Environment variables and CLI arguments | `env.md` |
| `error` | Structured errors and guards | `error.md` |
| `faker` | Fake data | `faker.md` |
| `fio` | Files and directories | `fio.md` |
| `fmt` | `printf`-style formatting | `fmt.md` |
| `hashable` | `Hashable` base struct | `hashable.md` |
| `http` | HTTP client, request parsing, response building | `http.md` |
| `httpserver` | HTTP server: routing, middleware, static files, CORS | `httpserver.md` |
| `web` | Web app interface: `call(env)` apps, middleware, builder/mounting, streaming, test client | `web.md` |
| `ipc` | Named-pipe messaging | `ipc.md` |
| `iter` | `ListIterator` cursor | `iter.md` |
| `json` | JSON parse/stringify, typed pipelines | `json.md` |
| `list` | List predicates, as list methods | `list.md` |
| `log` | Leveled logging to stderr/file | `log.md` |
| `math` | Math functions, random numbers | `math.md` |
| `path` | `Path` struct | `path.md` |
| `pipeline` | Named step pipelines | `pipeline.md` |
| `protobuf` | Protocol Buffers (proto3) | `protobuf.md` |
| `reflector` | Runtime introspection, `Callable` | `reflector.md` |
| `regex` | Regular expressions (pattern first) | `regex.md` |
| `signal` | `SIGINT`/`SIGTERM`/`SIGHUP` handlers | `signal.md` |
| `smtp` | Send email | `smtp.md` |
| `socket` / `tls` | Async TCP and TLS sockets | `socket.md` |
| `sqlite` | SQLite via `SqliteDb` | `sqlite.md` |
| `stat` | Descriptive statistics | `stat.md` |
| `std` | Small type-check and range helpers | `std.md` |
| `stdin` | Read standard input | `stdin.md` |
| `stopwatch` | `Stopwatch` with laps | `stopwatch.md` |
| `string` | String helpers, as string methods | `string.md` |
| `sys` | Run programs, manage processes | `sys.md` |
| `table` | Query lists of hashmaps: filter, join, group | `table.md` |
| `task` | Tasks, channels, mutexes, timers | `task.md` |
| `template` | `{{key}}` templates | `template.md` |
| `tester` | Unit tests | `tester.md` |
| `text` | Line-oriented text pipelines | `text.md` |
| `time` | Clock, timestamps, calendar helpers | `time.md` |
| `udp` | UDP datagrams | `udp.md` |
| `watch` | Watch files/directories for changes | `watch.md` |
| `xml` | XML parse/query/build, pipelines | `xml.md` |
| `yaml` | YAML parse/stringify | `yaml.md` |
