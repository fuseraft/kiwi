# `regex`

The `regex` package provides functions for working with regular expressions. All functions follow a **pattern-first** convention.

The underlying engine is .NET's `System.Text.RegularExpressions`, which uses the [.NET regex syntax](https://learn.microsoft.com/en-us/dotnet/standard/base-types/regular-expression-language-quick-reference).

The package also exposes three new string methods that can be called directly on any string value:
[`named_captures`](#named_captures), [`scan_groups`](#scan_groups), [`regex_escape`](#regex_escape).

---

## Package Functions

### `test(pattern, str)`

Returns `true` if the pattern matches anywhere in the string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to test. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if a match is found. |

**Example**

```kiwi
import "regex"

println regex::test('\d+', "order 42")     # true
println regex::test('\d+', "no numbers")   # false
println regex::test('^hello', "hello!")    # true
```

---

### `find(pattern, str)`

Returns the first matching substring, or `""` if there is no match.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to search. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The first match, or an empty string. |

**Example**

```kiwi
import "regex"

println regex::find('\d+', "price: 42 usd")   # 42
println regex::find('\d+', "no numbers")      # (empty string)
```

---

### `match(pattern, str)`

Returns the positional capture groups from the first match as a list of strings. The full match (group 0) is excluded — only capture groups `(...)` are returned. Returns an empty list if there is no match or no groups.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to search. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | Capture group strings from the first match. |

**Example**

```kiwi
import "regex"

groups = regex::match('(\d{4})-(\d{2})-(\d{2})', "date: 2025-03-15")
println groups[0]   # 2025
println groups[1]   # 03
println groups[2]   # 15
```

---

### `named_captures(pattern, str)`

Returns a hashmap of named capture groups from the first match. Use `(?<name>...)` syntax in the pattern to create named groups. Returns an empty hashmap if there is no match or the pattern has no named groups.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern with named groups. |
| `string` | `str` | The string to search. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap mapping group names to matched values. |

**Example**

```kiwi
import "regex"

m = regex::named_captures('(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})', "2025-03-15")
println m["year"]    # 2025
println m["month"]   # 03
println m["day"]     # 15
```

Also callable as a string method:

```kiwi
m = "2025-03-15".named_captures('(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})')
```

---

### `scan(pattern, str)`

Returns all non-overlapping full matches as a list of strings.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to search. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | All matching substrings in order. |

**Example**

```kiwi
import "regex"

nums = regex::scan('\d+', "a1 b22 c333")
println nums   # ["1", "22", "333"]

words = regex::scan('[A-Z][a-z]+', "Hello World Foo")
println words  # ["Hello", "World", "Foo"]
```

---

### `scan_groups(pattern, str)`

Returns the capture groups for every match in the string. Each item in the returned list is itself a list of capture group strings for one match (group 0 excluded, consistent with `regex::match`).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to search. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of lists, one inner list of capture groups per match. |

**Example**

```kiwi
import "regex"

pairs = regex::scan_groups('(\w+)=(\w+)', "x=1 y=2 z=3")

for p in pairs do
  println p[0] + " -> " + p[1]
end
# x -> 1
# y -> 2
# z -> 3
```

Also callable as a string method:

```kiwi
pairs = "x=1 y=2".scan_groups('(\w+)=(\w+)')
```

---

### `replace(pattern, str, replacement)`

Replaces every match of the pattern with the replacement string. Supports .NET backreferences in the replacement (`$1`, `$2`, `${name}`).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to modify. |
| `string` | `replacement` | The replacement text. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A new string with all matches replaced. |

**Example**

```kiwi
import "regex"

println regex::replace('\d+', "a1 b2 c3", "N")          # a_N_ b_N_ c_N_
println regex::replace('\s+', "too  many   spaces", " ") # too many spaces

# backreference: swap first and last name
println regex::replace('(\w+)\s(\w+)', "John Smith", '$2, $1')
# Smith, John
```

---

### `split(pattern, str, limit)`

Splits the string around matches of the pattern. Empty strings at the ends of the result are removed when no limit is given.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | The regular expression pattern. |
| `string` | `str` | The string to split. |
| `integer` | `limit` | Maximum number of substrings to return. Defaults to `-1` (no limit). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of substrings. |

**Example**

```kiwi
import "regex"

parts = regex::split('\s+', "one  two   three")
println parts   # ["one", "two", "three"]

# limit the number of splits
head = regex::split(',\s*', "a, b, c, d", 2)
println head   # ["a", "b, c, d"]
```

---

### `escape(str)`

Escapes all regex special characters in a string so it can be used as a literal match pattern.

Special characters escaped: `\ . ^ $ * + ? ( ) [ ] { } |`

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `str` | The string to escape. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The escaped string, safe to use as a literal regex pattern. |

**Example**

```kiwi
import "regex"

pattern = regex::escape("hello.world+foo?")
println regex::test(pattern, "hello.world+foo?")   # true
println regex::test(pattern, "helloxworldyfooZ")   # false
```

Also callable as a string method:

```kiwi
pattern = "3.14".regex_escape()   # "3\.14"
```

---

## String Methods

The following methods are available on any `string` value directly:

| Method | Equivalent package call |
| :--- | :--- |
| `str.find(pattern)` | `regex::find(pattern, str)` |
| `str.match(pattern)` | `regex::match(pattern, str)` |
| `str.matches(pattern)` | full-string match test |
| `str.matches_all(pattern)` | test entire string is covered by matches |
| `str.scan(pattern)` | `regex::scan(pattern, str)` |
| `str.scan_groups(pattern)` | `regex::scan_groups(pattern, str)` |
| `str.named_captures(pattern)` | `regex::named_captures(pattern, str)` |
| `str.rreplace(pattern, repl)` | `regex::replace(pattern, str, repl)` |
| `str.rsplit(pattern, limit?)` | `regex::split(pattern, str, limit?)` |
| `str.regex_escape()` | `regex::escape(str)` |
