# string

> String formatting and inspection helpers, available as string methods.

The `string` package adds formatting and inspection helpers to strings. Because the package [extends the `string` type](../language/packages.md#extending-built-in-types), every function can be called either way:

```kiwi
println string::titleize("the quick fox")   # The Quick Fox
println "the quick fox".titleize()           # The Quick Fox
```

The core string methods, such as `.split`, `.trim` and `.replace`, are [built in](../reference/builtin-methods.md#strings); this package covers the rest.

## Formatting

```kiwi
println "hELLO wORLD".capitalize()        # Hello world
println "hELLO wORLD".titleize()          # Hello World
println "  too   much   space ".squish()  # too much space
println "42".padstart(6, "0")             # 000042
println "ab".padend(5, ".")               # ab...
println "hi".center(8, "*")               # ***hi***
println "Hello, World!".slug()            # hello-world
```

`dedent` removes the indentation shared by every line, which tidies multi-line string literals:

```kiwi
fn usage()
  "
    Usage: tool [options]
      -h  show help
  ".dedent().trim()
end

println usage()
```

## Inspecting

```kiwi
println "12345".is_numeric()        # true
println "abc".is_alpha()            # true
println "abc123".is_alphanumeric()  # true
println "ABC".is_upper()            # true
println "abc".is_lower()            # true
println "hello world".contains_any(["xyz", "wor"])   # true
```

## Encoding

```kiwi
println "kiwi".base64encode()       # a2l3aQ==
println "a2l3aQ==".base64decode()   # kiwi
println "a b&c".urlencode()         # a+b%26c
println "a+b%26c".urldecode()       # a b&c
```

## Odds and ends

```kiwi
println "ab".echo(3)            # aaabbb
println "abc".interweave("123") # a1b2c3
println "kiwi".mirror()         # kiwiiwik
println "report.txt".to_path().ext()   # .txt (see the path package)
```

`shuffle` returns the characters in random order.

## Functions

### `string::contains_any`

```kiwi
string::contains_any(str, matches = [])
```

Returns `true` if `str` contains any values in `matches`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `str` | `String` |  | The input string.  |
| `matches` | `List` | `[]` | The strings to match against.  |

**Returns** `Boolean` — True if found.

### `string::base64encode`

```kiwi
string::base64encode(_input: string|list|bytes): string
```

Encodes data as a base64 string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `string\|list\|bytes` |  | The input string. |

**Returns** `string` — The base64 encoded string.

### `string::base64decode`

```kiwi
string::base64decode(_input: string): string
```

Decodes a base64 string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `string` |  | The input string.  |

**Returns** `string` — The decoded string.

### `string::urlencode`

```kiwi
string::urlencode(_input)
```

Encodes a string as a URL encoded string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `String` — The URL encoded string.

### `string::urldecode`

```kiwi
string::urldecode(_input)
```

Decodes a URL encoded string to plaintext.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string.  |

**Returns** `String` — The plaintext string.

### `string::padstart`

```kiwi
string::padstart(_input, _size, _padding = " ")
```

Pads the left side of a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string.  |
| `_size` | `Integer` |  | The max length of the output string. |
| `_padding` | `String` | `" "` | The padding content. |

**Returns** `String` — The padded string.

### `string::padend`

```kiwi
string::padend(_input, _size, _padding = " ")
```

Pads the right side of a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string.  |
| `_size` | `Integer` |  | The max length of the output string. |
| `_padding` | `String` | `" "` | The padding content. |

**Returns** `String` — The padded string.

### `string::center`

```kiwi
string::center(_input, _width, _padding = " ")
```

Centers a string within a given width.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |
| `_width` | `Integer` |  | The width of the resulting string. |
| `_padding` | `String` | `" "` | The padding character (default is space). |

**Returns** `String` — The centered string.

### `string::echo`

```kiwi
string::echo(_input, _count)
```

Repeats each character in the string a specified number of times.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |
| `_count` | `Integer` |  | The number of times each character is repeated. |

**Returns** `String` — The string with characters echoed.

### `string::interweave`

```kiwi
string::interweave(_input1, _input2)
```

Interweaves characters from two strings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input1` | `String` |  | The first input string. |
| `_input2` | `String` |  | The second input string. |

**Returns** `String` — The interwoven string.

### `string::mirror`

```kiwi
string::mirror(_input, _delimiter = "")
```

Creates a mirrored version of the input string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |
| `_delimiter` | `String` | `""` | The delimiter to separate the original string from its mirror. |

**Returns** `String` — The mirrored string.

### `string::shuffle`

```kiwi
string::shuffle(_input)
```

Randomly shuffles the characters in a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `String` — The shuffled string.

### `string::slug`

```kiwi
string::slug(_input)
```

Generates a URL-friendly slug from a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `String` — The generated slug.

### `string::capitalize`

```kiwi
string::capitalize(_input: string): string
```

Returns a copy of the string with the first character uppercased and the rest lowercased.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `string` |  | The input string. |

**Returns** `String` — The capitalized string.

### `string::titleize`

```kiwi
string::titleize(_input: string): string
```

Returns a copy of the string with the first character of every word uppercased and the rest lowercased.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `string` |  | The input string. |

**Returns** `String` — The capitalized string.

### `string::smart_titleize`

```kiwi
string::smart_titleize(s: string, keep_lowercase: list = ["a", "an", "and", "but", "by", "for", "in", "nor", "of", "on", "or", "the", "to", "with"]): string
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string` |  |  |
| `keep_lowercase` | `list` | `["a", "an", "and", "but", "by", "for", "in", "nor", "of", "on", "or", "the", "to", "with"]` |  |

**Returns** `string`

### `string::is_numeric`

```kiwi
string::is_numeric(_input): boolean
```

Checks if the string consists only of numeric characters.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `Boolean` — True if the string is numeric, otherwise false.

### `string::is_alpha`

```kiwi
string::is_alpha(_input)
```

Checks if the string consists only of alphabetic characters.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `Boolean` — True if the string is alphabetic, otherwise false.

### `string::is_alphanumeric`

```kiwi
string::is_alphanumeric(_input)
```

Checks if the string consists only of alphanumeric characters.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `Boolean` — True if the string is alphanumeric, otherwise false.

### `string::is_upper`

```kiwi
string::is_upper(_input)
```

Checks if the string is entirely in uppercase.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `Boolean` — True if the string is all uppercase, false otherwise.

### `string::is_lower`

```kiwi
string::is_lower(_input)
```

Checks if the string is entirely in lowercase.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `Boolean` — True if the string is all lowercase, false otherwise.

### `string::to_upper`

```kiwi
string::to_upper(_input)
```

Converts all characters in the string to uppercase.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `String` — The string converted to uppercase.

### `string::to_lower`

```kiwi
string::to_lower(_input)
```

Converts all characters in the string to lowercase.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `String` |  | The input string. |

**Returns** `String` — The string converted to lowercase.

### `string::dedent`

```kiwi
string::dedent(text: string): string
```

Remove common leading whitespace from multi-line strings

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `text` | `string` |  | The input string |

**Returns** `String` — The dedented string.

### `string::to_path`

```kiwi
string::to_path(path: string): Path
```

Gives the string type a function that returns itself as an instance of Path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | The path. |

**Returns** `Path` — The path as a Path.

### `string::squish`

```kiwi
string::squish(_input: string): string
```

Strips leading/trailing whitespace and collapses internal whitespace runs to a single space.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_input` | `string` |  | The string to normalize. |

**Returns** `string` — The normalized string.

```kiwi
string::squish("  hello   world  ")    # "hello world"
string::squish("  7800    BEEMAN   AV") # "7800 BEEMAN AV"
```

## `StringIterator` struct

Reads a string one character at a time, for hand-written tokenizers and parsers.

### `StringIterator.new`

```kiwi
StringIterator.new(s: string)
```

Creates an iterator positioned at the first character.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string` |  | The string to read. |

### `.consume`

```kiwi
consume(): string
```

Advances past the current character.

**Returns** `string` — The character consumed, or `""` at the end.

### `.skip`

```kiwi
skip(n: integer)
```

Advances `n` characters, stopping at the end.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` |  | The number of characters to skip. |

### `.can_read`

```kiwi
can_read(): boolean
```

Checks whether a character is left to read.

**Returns** `boolean` — `true` while the position is before the end.

### `.eof`

```kiwi
eof(): boolean
```

Checks whether the whole string has been read.

**Returns** `boolean` — `true` at the end.

### `.current`

```kiwi
current(): string
```

Gets the character at the current position.

**Returns** `string` — The current character, or `""` at the end.

### `.peek`

```kiwi
peek(): string
```

Gets the character after the current one without consuming anything.

**Returns** `string` — The next character, or `""` if there is none.

### `.position`

```kiwi
position(): integer
```

Gets the current position.

**Returns** `integer` — The index of the current character.

### `.length`

```kiwi
length(): integer
```

Gets the length of the string.

**Returns** `integer` — The number of characters.

### `.rewind`

```kiwi
rewind(n: integer = 1)
```

Moves back `n` characters, stopping at the start.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` | `1` | The number of characters to move back (default `1`). |

### `.grab`

```kiwi
grab(n: integer): string
```

Gets the character at an absolute index.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` |  | The index. |

**Returns** `string` — The character, or `""` if `n` is out of range.

### `.reach`

```kiwi
reach(n: integer): string
```

Gets a character counted from the end of the string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` |  | A negative offset from the end (`-1` is the last character). |

**Returns** `string` — The character, or `""` if the offset is out of range.

### `.look_ahead`

```kiwi
look_ahead(): string
```

Gets the next character that isn't whitespace, from the current position on, without consuming anything.

**Returns** `string` — The character, or `""` if only whitespace remains.

### `.count_ahead`

```kiwi
count_ahead(): integer
```

Counts the whitespace characters from the current position to the next non-whitespace character.

**Returns** `integer` — The number of whitespace characters ahead.

### `.skip_whitespace`

```kiwi
skip_whitespace()
```

Advances past any whitespace (spaces, tabs and line breaks).
