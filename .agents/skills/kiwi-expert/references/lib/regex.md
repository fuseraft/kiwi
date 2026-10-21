# regex

> Regular expression matching, searching, splitting and replacing.

The `regex` package wraps the .NET regular expression engine. Every function takes the **pattern first**, then the text. Write patterns as single-quoted raw strings so backslashes reach the engine unchanged.

```kiwi
println regex::test('^\d{3}-\d{4}$', "555-1234")                # true
println regex::find('\d+', "order 66 of 99")                     # 66
println regex::scan('\d+', "1 22 333")                           # ["1", "22", "333"]
println regex::match('(\w+)@(\w+)\.com', "mail ada@example.com") # ["ada", "example"]
println regex::replace('\s+', "too   many    spaces", " ")       # too many spaces
println regex::split('[,;]\s*', "a, b;c")                        # ["a", "b", "c"]
```

## Groups

`match` returns the capture groups of the first match. `scan_groups` returns them for every match, and `named_captures` returns named groups as a hashmap:

```kiwi
println regex::scan_groups('(\w)=(\d)', "a=1 b=2")
# [["a", "1"], ["b", "2"]]

println regex::named_captures('(?<user>\w+)@(?<host>[\w.]+)', "ada@example.com")
# {"user": "ada", "host": "example.com"}
```

In a replacement, `$1`, `$2` and so on refer to groups:

```kiwi
println regex::replace('(\d{4})-(\d{2})-(\d{2})', "2024-06-15", "$3/$2/$1")   # 15/06/2024
```

## Patterns

Patterns use [.NET regular expression syntax](https://learn.microsoft.com/en-us/dotnet/standard/base-types/regular-expression-language-quick-reference). Inline options such as `(?i)` for case-insensitive matching work:

```kiwi
println regex::test('(?i)^kiwi$', "KIWI")   # true
```

To match text literally, escape it first:

```kiwi
println regex::escape("1+1=2?")   # 1\+1=2\?
```

## As string methods

Strings have the same operations as [built-in methods](../reference/builtin-methods.md#regular-expressions), with the text first: `text.matches(pattern)`, `text.find(pattern)`, `text.scan(pattern)`, `text.rreplace(pattern, replacement)`, `text.rsplit(pattern)` and so on.

## Functions

### `regex::test`

```kiwi
regex::test(pattern: string, str: string): boolean
```

Returns `true` if the pattern matches anywhere in the string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to test. |

**Returns** `boolean` — `true` if a match is found, `false` otherwise.

### `regex::find`

```kiwi
regex::find(pattern: string, str: string): string
```

Returns the first substring matching the pattern, or `""` if there is no match.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to search. |

**Returns** `string` — The first matching substring, or an empty string.

### `regex::match`

```kiwi
regex::match(pattern: string, str: string): list
```

Returns the capture groups from the first match as a list. Group 0 (the full match) is excluded — only named or positional capture groups are returned. Returns an empty list if there is no match or no capture groups.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to search. |

**Returns** `list` — A list of capture group strings from the first match.

### `regex::named_captures`

```kiwi
regex::named_captures(pattern: string, str: string): hashmap
```

Returns a hashmap of named capture groups from the first match. Returns an empty hashmap if there is no match or the pattern has no named groups.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern (use `(?<name>...)` for named groups). |
| `str` | `string` |  | The string to search. |

**Returns** `hashmap` — A hashmap mapping group names to their matched values.

### `regex::scan`

```kiwi
regex::scan(pattern: string, str: string): list
```

Returns all non-overlapping matches as a list of strings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to search. |

**Returns** `list` — A list of matched substrings.

### `regex::scan_groups`

```kiwi
regex::scan_groups(pattern: string, str: string): list
```

Returns the capture groups for every match in the string. Each item in the returned list is itself a list of capture group strings for one match (group 0 excluded, consistent with `regex::match`).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to search. |

**Returns** `list` — A list of lists of capture group strings, one inner list per match.

### `regex::replace`

```kiwi
regex::replace(pattern: string, str: string, replacement: string): string
```

Replaces every match of the pattern with the replacement string. Supports backreferences in the replacement (e.g. `$1`, `${name}`).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to modify. |
| `replacement` | `string` |  | The replacement text. |

**Returns** `string` — A new string with all matches replaced.

### `regex::split`

```kiwi
regex::split(pattern: string, str: string, limit: integer = -1): list
```

Splits the string around matches of the pattern.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | The regular expression pattern. |
| `str` | `string` |  | The string to split. |
| `limit` | `integer` | `-1` | Maximum number of substrings to return. Use `-1` for no limit (default). |

**Returns** `list` — A list of substrings.

### `regex::escape`

```kiwi
regex::escape(str: string): string
```

Escapes all regex special characters in a string so it can be used as a literal pattern.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `str` | `string` |  | The string to escape. |

**Returns** `string` — The escaped string.

```kiwi
pattern = regex::escape("hello.world")
println regex::test(pattern, "hello.world")   # true
println regex::test(pattern, "helloxworld")   # false
```
