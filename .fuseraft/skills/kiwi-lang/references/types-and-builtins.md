# Kiwi Types and Builtins Reference

This file consolidates the 13 builtin types (from `docs/types.md`) with their associated dot-notation builtins (from `docs/builtins.md`). Use `typeof(x)` for runtime type names.

---

## Type Overview

| Type | Description | Key Notes |
|------|-------------|-----------|
| `boolean` | `true` / `false` | `null` coerces to `false`; only `0` is falsy among numbers |
| `bytes` | Byte array | Created via `[33..126].to_bytes()` or string `.to_bytes()` |
| `date` | Date-time value | Parse with `.to_date()`; methods: `.year()`, `.month()`, etc. |
| `float` | 64-bit float | `3.14159`; many numeric builtins apply |
| `generator` | Lazy sequence | Produced by `yield`; iterable with `for` |
| `hashmap` | Key-value dictionary | `{ "k": v }`; dot or bracket access; mutable |
| `integer` | 64-bit signed | `42`; most common numeric type |
| `lambda` | Anonymous function | `with (x) do … end`; callable like `fn` |
| `function` | Named function reference | `typeof(myfn)` returns "function"; distinct from "lambda" |
| `list` | Ordered collection | `[1, 2, 3]`; mutable, indexable, rich iteration API |
| `null` | Absence of value | Falsy; explicit `null` literal |
| `object` | Struct instance | Created with `StructName.new()`; `@` fields inside methods |
| `pointer` | Internal managed reference | Used by channel builtins (`__chan_*__`) |
| `string` | Mutable text | Rich surface: `.trim()`, `.split()`, regex, etc. |

Introspection: `typeof(x)` (global) or `x.type()` (method) both return the type name string.

---

## Date Builtins

- `.year()`, `.month()`, `.day()`, `.hour()`, `.minute()`, `.second()`, `.millisecond()`

Example:
```kiwi
dt = "2024-06-15".to_date()
println dt.year()  # 2024
```

---

## String Builtins

- `.begins_with(str)`, `.ends_with(str)`, `.contains(str)`, `.index(str)`, `.lastindex(str)`
- `.chars()`, `.lines()`, `.to_list()`, `.to_bytes()`, `.to_hex()`
- `.trim()`, `.ltrim()`, `.rtrim()`, `.chomp()`
- `.uppercase()`, `.lowercase()`
- `.substring(pos, length?)`
- `.ord()` — Unicode code point of first char
- Regex: `.find(regex)`, `.match(regex)`, `.matches(regex)`, `.matches_all(regex)`, `.replace(search, repl)`, `.rreplace(...)`, `.rsplit(...)`, `.scan(regex)`, `.scan_groups(regex)`, `.named_captures(regex)`, `.regex_escape()`
- Conversion: `.to_integer()`, `.to_float()`, `.to_date(format?)`, `.to_string(format?)`

**Character classification** (common pattern using `.ord()`):
```kiwi
fn is_whitespace(ch) return ch.ord() <= 32 or ch == "\n" or ch == "\r" end
fn is_alpha(ch)      c = ch.ord(); return (c >= 65 and c <= 90) or (c >= 97 and c <= 122) end
fn is_digit(ch)      c = ch.ord(); return c >= 48 and c <= 57 end
fn is_alphanum(ch)   return is_alpha(ch) or is_digit(ch) end
```

Global: `serialize(value)`, `deserialize(str)`

---

## Hashmap Builtins

- `.keys()`, `.values()`, `.size()`
- `.has_key(key)`, `.get(key, default?)`, `.set(key, value)`, `.remove(key)`
- `.merge(other)` — other overrides
- `.clear()`

Global conversion: `x.pretty()` for indented JSON-like output.

---

## List Builtins

Core mutation & access:
- `.push(v)`, `.pop()`, `.append(v)`, `.enqueue(v)`, `.dequeue()`, `.shift()`, `.unshift(v)`
- `.insert(index, v)`, `.remove(v)`, `.remove_at(index)`, `.swap(i1, i2)`, `.clear()`
- `.size()`, `.first(default?)`, `.last(default?)`, `.index(v)`, `.lastindex(v)`

Iteration & transformation:
- `.each(lambda)`, `.map(lambda)`, `.filter(lambda)`, `.select` (alias), `.reduce(acc, lambda)`
- `.all(lambda)`, `.none(lambda)`, `.count(v)`, `.sum()`
- `.sort()`, `.sort(comparator_lambda)`, `.reverse()`, `.unique()`, `.flatten()`
- `.concat(other)`, `.zip(other)`, `.group_by_field(field)`
- `.skip(n)`, `.take(n)`, `.slice(start, end)`

Conversion:
- `.to_bytes()`, `.to_hex()`, `.join(sep)`, `.to_list()` (alias for chars on strings)

---

## Numeric Builtins (integer / float)

- `.between(a, b)` — inclusive range check (also works on strings/dates)
- `.empty()` — true for `0` / `0.0`
- `.truthy()` — `0`/`0.0` are falsy; everything else truthy
- `.to_integer()`, `.to_float()`, `.to_string(format?)` (see formatting table in builtins.md)
- `.clone()`

---

## Boolean / Null Builtins

- `.truthy()`, `.empty()`, `.to_string()`
- `null` is never truthy; `!null` → true in conditionals

---

## Global (Free) Builtins

These are called without dot notation:

| Builtin | Purpose |
|---------|---------|
| `typeof(x)` | Runtime type name (recommended over `.type()`) |
| `println(...)`, `print(...)` | Output |
| `input(prompt?)` | Read line from stdin |
| `range(...)` | Not directly listed; use `[a..b]` syntax or list conversion |
| `exit(code?)` | Terminate |
| `raise(msg)` | Throw error |
| `serialize(v)`, `deserialize(s)` | JSON-like round-trip |
| `__execpath__()`, `__entrypath__()` | Runtime paths |
| `__memusage__()` | Memory diagnostics |
| `__tokenize__(code)` | Lexer introspection |
| `__chan_create__`, `__chan_send__`, etc. | Internal channel primitives (pointer type) |

---

## Conversion & Introspection (all types)

- `.type()` → type name string
- `.is_a(type_name_or_struct)` → inheritance-aware for structs
- `.clone()` → deep copy
- `.empty(default?)` → zero-like check with optional default
- `.truthy()` → boolean truthiness
- `.to_string(format?)`, `.to_integer()`, `.to_float()`, `.to_date(...)`, `.to_list()`, `.to_bytes()`, `.pretty()`

---

## Notes for Agents

- **Builtin vs stdlib**: Dot-notation methods above are always available (no import). Stdlib functions require `import pkg` and are called as `pkg.func(...)`.
- **Size/length**: There is **no** `.length()` builtin. Use `.size()` for strings, lists, and hashmaps.
- **No `.toString()`**: Lists, strings, and other values coerce automatically when concatenated with `+` or passed to `println`/`print`. Use `.to_string()` only when an explicit string conversion is needed.
- **Naming convention**: Functions and struct methods must use `snake_case` (e.g. `parse_select`, `is_keyword`). CamelCase identifiers are not valid for these.
- **Loop control**: Use `next` (optionally with `when` guard) instead of `continue`.
- **Null literal**: The only absence value is `null` (never `nil`).
- When in doubt about a method, check `docs/builtins.md` or the per-package docs under `docs/lib/`.
- `typeof(x)` is the most reliable way to inspect dynamic types in conditionals or debugging.

---

## Lessons from tsql_parser.kiwi (verified working parser example)

A minimal recursive-descent TSQL parser (`tsql_parser.kiwi`) was used to validate these rules. It successfully:

- Tokenizes and parses `SELECT col1, col2 FROM table WHERE cond`
- Builds lists for columns and WHERE conditions using `push`
- Uses `null` checks and `is_keyword` helper
- Demonstrates `snake_case` method naming (`parse_select`, `is_keyword`)
- Consumes tokens with a simple `current()` / `advance()` / `expect()` pattern

This example is now part of the regression set for the Kiwi language reference.

---

## Docstrings

Kiwi supports documentation strings using triple double-quotes (`"""`).

**Placement rules (verified):**
- File-level docstring: first statement in the file (before any code or comments).
- Function docstring: immediately before `fn name(...)`.
- Struct docstring: immediately before `struct Name`.
- Method docstring: immediately before `func name(self, ...)` inside a struct.

**Style:**
- First line should be a brief summary.
- Follow with a blank line and then detailed description if needed.
- Use backticks for code elements.

These conventions were validated while adding documentation to `tsql_parser.kiwi`.

**Example (from tsql_parser.kiwi):**
```kiwi
"""
TSQL Parser — minimal tokenizer + SELECT parser.

Demonstrates docstring usage, qualified-name handling (alias.column),
and the `null` / `next` / `snake_case` patterns.
"""

struct Parser
    """
    Simple token-stream parser for a minimal TSQL SELECT statement.
    """
    ...
```