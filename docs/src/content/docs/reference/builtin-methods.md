---
title: Built-in Methods
description: Every method available on Kiwi's built-in types, grouped by type.
---

Built-in methods are called with a dot on any value: `"kiwi".uppercase()`, `[3, 1, 2].sort()`. They are part of the interpreter, so they are always available.

To call a method on a number literal, wrap it in parentheses: `(42).to_string()`.

Packages can add more methods to built-in types; see [Extending built-in types](/kiwi/language/packages/#extending-built-in-types). The standard library does this for [strings](#from-the-string-package), [lists](#from-the-list-package) and [dates](#from-the-date-package). A built-in method always wins over an extension with the same name.

## All values

| Method | Returns |
| :-- | :-- |
| `.type()` | The type name, or the struct name for an instance. Same as `typeof(value)`. |
| `.is_a(type)` | Whether the value has that type. Takes a type keyword, a type name string, or a struct. |
| `.to_string(format?)` | The value as a string. Numbers accept a [format code](/kiwi/language/types/#number-formats). |
| `.truthy()` | The value's [truthiness](/kiwi/language/types/#truthiness) as a boolean. |
| `.empty(default?)` | Whether the value is falsy (`0`, `""`, `[]`, `{}`, `false`, `null`). With a default, returns the default when empty and the value otherwise. |
| `.clone()` | A deep copy of a list or hashmap, or a copy of a struct instance. |
| `.pretty()` | An indented, JSON-style string. |

```kiwi
println (0).empty()              # true
println "".empty("anonymous")    # anonymous
println "Ada".empty("anonymous") # Ada
println [1, 2].is_a(list)        # true
println (3.14159).to_string("f2")   # 3.14
```

## Numbers

| Method | Returns |
| :-- | :-- |
| `.to_integer()` | The number truncated toward zero. |
| `.to_float()` | The number as a float. |
| `.to_string(format?)` | A string; see [number formats](/kiwi/language/types/#number-formats). |
| `.between(low, high)` | Whether the number is in the inclusive range. |
| `.clamp(low, high)` | The number limited to the range. |
| `.to_date()` | A date, treating the integer as a Unix timestamp in milliseconds. |

```kiwi
println (15).clamp(0, 10)      # 10
println (5).between(1, 10)     # true
println (-3.7).to_integer()    # -3
println (255).to_string("X")   # FF
```

For rounding, absolute values, powers and so on, see the [`math`](/kiwi/stdlib/math/) package.

## Strings

### Inspecting

| Method | Returns |
| :-- | :-- |
| `.size()`, `.length()` | The number of characters. |
| `.contains(text)` | Whether `text` occurs in the string. |
| `.begins_with(text)` | Whether the string starts with `text`. |
| `.ends_with(text)` | Whether the string ends with `text`. |
| `.index(text)` | The position of the first occurrence, or `-1`. |
| `.lastindex(text)` | The position of the last occurrence, or `-1`. |
| `.count(text)` | How many times `text` occurs. |
| `.between(low, high)` | Whether the string sorts between `low` and `high`, inclusive. |
| `.ord()` | The Unicode code point of the first character. |

### Transforming

| Method | Returns |
| :-- | :-- |
| `.uppercase()`, `.lowercase()` | The string in upper or lower case. |
| `.trim()`, `.ltrim()`, `.rtrim()` | The string without surrounding whitespace, on both, the left or the right side. |
| `.chomp()` | The string without one trailing line break. |
| `.replace(find, with)` | The string with every occurrence of the text `find` replaced. |
| `.substring(start, length?)` | `length` characters from `start`, or the rest of the string. |
| `.reverse()` | The characters in reverse order. |
| `.concat(text)` | The two strings joined. |

### Splitting and converting

| Method | Returns |
| :-- | :-- |
| `.split(separator)` | A list of the parts between occurrences of `separator`. An empty separator splits into characters. |
| `.lines()` | A list of lines. |
| `.chars()`, `.to_list()` | A list of characters. |
| `.to_integer()`, `.to_float()` | The parsed number. Throws `ConversionError` if the string isn't numeric. |
| `.to_date(format?)` | The parsed date, optionally with an exact [format](#date-formats). |
| `.to_bytes()` | The UTF-8 bytes. |
| `.hex_bytes()` | Bytes parsed from hexadecimal text; spaces and tabs are ignored. |

```kiwi
s = "Hello, World"
println s.substring(7)            # World
println s.substring(0, 5)         # Hello
println s.lastindex("o")          # 8
println s.count("l")              # 3
println "a,b,,c".split(",")       # ["a", "b", "", "c"]
println "line 1\nline 2".lines()  # ["line 1", "line 2"]
```

### Regular expressions

Pass patterns as raw (single-quoted) strings.

| Method | Returns |
| :-- | :-- |
| `.matches(pattern)` | Whether the whole string matches. |
| `.matches_all(pattern)` | Whether the pattern matches anywhere in the string. |
| `.find(pattern)` | The first match, or `""`. |
| `.match(pattern)` | The capture groups of the first match. |
| `.scan(pattern)` | Every match. |
| `.scan_groups(pattern)` | The capture groups of every match, as a list of lists. |
| `.named_captures(pattern)` | A hashmap of named groups from the first match. |
| `.rreplace(pattern, replacement)` | The string with every match replaced; `$1` refers to group 1. |
| `.rsplit(pattern, limit?)` | The parts between matches, at most `limit` of them. |
| `.regex_escape()` | The string with regex metacharacters escaped. |

```kiwi
log = "user=ada id=42 user=grace id=7"
println log.scan('id=(\d+)')                          # ["id=42", "id=7"]
println log.scan_groups('user=(\w+) id=(\d+)')        # [["ada", "42"], ["grace", "7"]]
println log.named_captures('user=(?<name>\w+)')       # {"name": "ada"}
println log.rreplace('id=\d+', "id=?")                # user=ada id=? user=grace id=?
println "2024-06-15".rreplace('(\d+)-(\d+)-(\d+)', "$3/$2/$1")   # 15/06/2024
println "k = v = w".rsplit('\s*=\s*', 2)              # ["k", "v = w"]
```

## Lists

### Reading

| Method | Returns |
| :-- | :-- |
| `.size()`, `.length()` | The number of elements. |
| `.first()`, `.last()` | The first or last element, or `null` if empty. |
| `.get(index)` | The element at `index`. |
| `.contains(value)` | Whether the list contains `value`. |
| `.index(value)`, `.lastindex(value)` | The first or last position of `value`, or `-1`. |
| `.count(value)` | How many elements equal `value`. |
| `.slice(start, end)` | The elements from `start` up to `end`. |
| `.take(n)`, `.skip(n)` | The first `n` elements, or everything after them. |
| `.min()`, `.max()`, `.sum()` | The smallest, largest, or total of the elements. |
| `.join(separator?)` | The elements joined into a string. |

### Changing

These change the list in place and return it.

| Method | Effect |
| :-- | :-- |
| `.push(value)`, `.append(value)`, `.enqueue(value)` | Add to the end. |
| `.unshift(value)` | Add to the front. |
| `.insert(index, value)` | Insert at `index`. |
| `.concat(list)` | Append every element of `list`. |
| `.set(index, value)` | Replace the element at `index`. |
| `.remove(value)` | Remove the first element equal to `value`. |
| `.remove_at(index)` | Remove the element at `index`. |
| `.clear()` | Remove everything. |
| `.sort()`, `.sort(comparator)` | Sort ascending, or by a comparator that returns `true` when its first argument comes first. |
| `.reverse()` | Reverse the order. |
| `.rotate(n)` | Rotate right by `n` (left if negative). |
| `.swap(i, j)` | Swap two elements. |

`.pop()` removes and returns the last element, and `.shift()` (or `.dequeue()`) removes and returns the first.

### Transforming

These return new values and leave the list unchanged.

| Method | Returns |
| :-- | :-- |
| `.map(fn)` | The results of calling `fn` on each element. |
| `.filter(fn)` | The elements for which `fn` is truthy. |
| `.reduce(initial, fn)` | The result of folding `fn(accumulator, element)` over the list. |
| `.each(fn)` | Calls `fn(element, index)` for each element. Returns `null`. |
| `.all(fn)` | Whether `fn` is truthy for every element. |
| `.unique()` | The elements without duplicates. |
| `.flatten()` | Nested lists flattened into one. |
| `.zip(list)` | A list of `[a, b]` pairs. |
| `.group_by_field(key)` | A hashmap from each distinct value of `key` to the hashmaps with that value. |
| `.to_bytes()` | Bytes from a list of integers. |
| `.to_hex()` | A hex string from a list of byte values. |

```kiwi
orders = [
  {region: "west", total: 120},
  {region: "east", total: 80},
  {region: "west", total: 50}
]

println orders.map(do (o) => o["total"]).sum()          # 250
println orders.filter(do (o) => o["total"] > 60).size()  # 2
println orders.group_by_field("region").keys()           # ["west", "east"]
println [1, [2, [3]]].flatten()                          # [1, 2, 3]
println [1, 2, 3].zip(["a", "b", "c"])                   # [[1, "a"], [2, "b"], [3, "c"]]
println [9, 4, 7].sort(do (a, b) => a > b)               # [9, 7, 4]
```

## Hashmaps

| Method | Returns |
| :-- | :-- |
| `.size()`, `.length()` | The number of entries. |
| `.keys()`, `.values()` | The keys or values, in insertion order. |
| `.has_key(key)` | Whether the key exists. |
| `.get(key, default?)` | The value for `key`, or `default` (or `null`) if it is missing. |
| `.set(key, value)` | Sets a key. Changes the hashmap and returns it. |
| `.remove(key)` | Removes a key. Changes the hashmap and returns it. |
| `.merge(other)` | Copies every entry of `other` in, overwriting existing keys. Changes the hashmap and returns it. |
| `.clear()` | Removes every entry. |

```kiwi
config = {host: "localhost"}
config.merge({port: 8080})
println config.get("port")           # 8080
println config.get("tls", false)     # false
println config.has_key("host")       # true
println config.keys()                # ["host", "port"]
```

## Dates

| Method | Returns |
| :-- | :-- |
| `.year()`, `.month()`, `.day()` | The calendar date parts. Months run from 1 to 12. |
| `.hour()`, `.minute()`, `.second()`, `.millisecond()` | The time of day parts. |
| `.to_string(format?)` | The date as text, optionally in a [format](#date-formats). |
| `.between(start, end)` | Whether the date falls in the inclusive range. |

Dates compare with `<`, `>` and `==`.

### Date formats

Format strings use .NET date patterns:

| Pattern | Meaning | Example |
| :-- | :-- | :-- |
| `yyyy`, `yy` | Year | `2024`, `24` |
| `MM`, `M`, `MMM`, `MMMM` | Month | `06`, `6`, `Jun`, `June` |
| `dd`, `d` | Day of the month | `05`, `5` |
| `ddd`, `dddd` | Day of the week | `Sat`, `Saturday` |
| `HH`, `hh` | Hour (24-hour, 12-hour) | `14`, `02` |
| `mm`, `ss`, `fff` | Minute, second, millisecond | `30`, `05`, `123` |
| `tt` | AM/PM | `PM` |

```kiwi
launch = "2024-06-15T14:30:00".to_date()
println launch.to_string("yyyy-MM-dd HH:mm")   # 2024-06-15 14:30
println launch.to_string("dddd, MMMM d")       # Saturday, June 15
println "15/06/2024".to_date("dd/MM/yyyy").year()   # 2024
```

## Bytes

| Method | Returns |
| :-- | :-- |
| `.size()` | The number of bytes. |
| `.to_string()` | The bytes decoded as UTF-8. |
| `.to_list()` | A list of integers from 0 to 255. |
| `.to_hex()` | A lowercase hexadecimal string. |
| `.slice(start, end)`, `.reverse()` | A portion of the bytes, or the bytes reversed. |

```kiwi
data = "Kiwi".to_bytes()
println data.to_hex()                # 4b697769
println data.to_list()               # [75, 105, 119, 105]
println "4b 69".hex_bytes().to_string()   # Ki
```

## Functions and lambdas

| Method | Returns |
| :-- | :-- |
| `.call(arguments)` | The result of calling the function with a list of arguments. |
| `.parameters()` | A list describing each parameter, with `name` and `position`. |

```kiwi
fn add(a, b)
  a + b
end

println add.call([2, 3])                                  # 5
println add.parameters().map(do (p) => p["name"])         # ["a", "b"]
```

## Struct instances

| Method | Returns |
| :-- | :-- |
| `.keys()` | The instance variable names. |
| `.values()` | The instance variable values. |
| `.set(name, value)` | Sets an instance variable by name. |
| `.clone()` | A copy of the instance. |
| `.is_a(Struct)` | Whether the instance is that struct or a descendant. |

See [Structs](/kiwi/language/structs/#working-with-instances).

## Enums

`.size()`, `.keys()`, `.values()`, `.to_hashmap()` and `.get(name, default)`. See [Enums](/kiwi/language/enums/#enum-methods).

## Methods from the standard library

Every function in the stdlib `string`, `list` and `date` packages can be called as a method, because those packages [extend built-in types](/kiwi/language/packages/#extending-built-in-types). Each is also callable as a package function.

### From the `string` package

Every `string::` function, for example `.titleize()`, `.capitalize()`, `.squish()`, `.dedent()`, `.slug()`, `.padstart(width, pad)`, `.padend(width, pad)`, `.center(width, pad)`, `.base64encode()`, `.is_numeric()` and `.to_path()`. See [`string`](/kiwi/stdlib/string/).

### From the `list` package

Every `list::` function, for example `.reject(fn)`, `.first_where(fn)`, `.last_where(fn)`, `.one(fn)`, `.is_unique()`, `.index_of(value)`, `.last_index_of(value)`, `.sum_integer()`, `.sum_float()` and `.iterator()`. The package's `all`, `any`, `none`, `count` and `find` share names with built-ins or keywords, so call them as `list::count(xs, fn)`. See [`list`](/kiwi/stdlib/list/).

### From the `date` package

Every `date::` function, for example `.add_days(n)`, `.add_months(n)`, `.add_years(n)`, `.add_hours(n)`, `.add_minutes(n)`, `.add_seconds(n)`, `.add_milliseconds(n)`, `.weekday()`, `.yearday()`, `.isdst()` and `.ampm()`. See [`date`](/kiwi/stdlib/date/).

```kiwi
release = "2024-06-15".to_date()
println release.add_days(20).to_string("yyyy-MM-dd")   # 2024-07-05
println release.weekday()                              # 6
println "  messy   text ".squish()                     # messy text
println [1, 2, 3, 4].reject(do (n) => n % 2 == 0)      # [1, 3]
```
