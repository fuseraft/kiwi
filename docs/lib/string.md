# `string`

The `string` package contains specialized functions for strings.

---

## Package Functions

### `base64decode(_input)`

Decodes a base64 string to plaintext.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The plaintext string. |

**Example**
```kiwi
import "string"

encoded = string::base64encode("Hello, Kiwi!")
println encoded                         # prints: SGVsbG8sIEtpd2kh
println string::base64decode(encoded)   # prints: Hello, Kiwi!
```

### `base64encode(_input)`

Encodes data as a base64 string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string`\|`list`\|`bytes` | `_input` | The input data. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The base64 encoded string. |

**Example**
```kiwi
import "string"

println string::base64encode("Hello, Kiwi!")        # prints: SGVsbG8sIEtpd2kh
println string::base64encode("Hello".to_bytes())    # encode from bytes
```

### `center(_input, _size, _padding = " ")`

Pads both sides of a string, centering it within a field of `_size` characters.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_size` | The total width of the output string. |
| `string` | `_padding` | The padding character. Defaults to a single space ` `. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The centered string. |

**Example**
```kiwi
import "string"

println string::center("hi", 10)        # prints: "    hi    "
println string::center("hi", 10, "-")   # prints: "----hi----"
println string::center("kiwi", 10, "*") # prints: "***kiwi***"
```

### `contains_any(str, matches = [])`

Returns `true` if `str` contains any of the substrings in `matches`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `str` | The input string. |
| `list` | `matches` | The substrings to search for. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` if any match is found. |

**Example**
```kiwi
import "string"

println string::contains_any("hello world", ["world", "earth"])  # prints: true
println string::contains_any("hello world", ["mars", "venus"])   # prints: false
println string::contains_any("foo@bar.com", ["@", "#"])          # prints: true
```

### `echo(_input, _count)`

Repeats each character in the string `_count` times.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_count` | The number of times to repeat each character. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The string with each character repeated. |

**Example**
```kiwi
import "string"

println string::echo("abc", 3)   # prints: aaabbbccc
println string::echo("hi", 2)    # prints: hhii
```

### `interweave(_input1, _input2)`

Interweaves two strings character by character, alternating one from each. If one string is shorter, the remaining characters of the longer string are appended.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input1` | The first input string. |
| `string` | `_input2` | The second input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The interwoven string. |

**Example**
```kiwi
import "string"

println string::interweave("abc", "123")    # prints: a1b2c3
println string::interweave("hi", "!!!")     # prints: h!i!!!
```

### `isalpha(_input)`

Checks if the string consists only of alphabetic characters (A–Z, a–z).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` if the string contains only letters. |

**Example**
```kiwi
import "string"

println string::isalpha("hello")    # prints: true
println string::isalpha("hello1")   # prints: false
println string::isalpha("héllo")    # prints: false
```

### `isalphanumeric(_input)`

Checks if the string consists only of alphanumeric characters (A–Z, a–z, 0–9).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` if the string contains only letters and digits. |

**Example**
```kiwi
import "string"

println string::isalphanumeric("hello123")   # prints: true
println string::isalphanumeric("hello!")     # prints: false
```

### `islower(_input)`

Checks if the string is entirely lowercase (no uppercase letters present).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` if the string has no uppercase letters. |

**Example**
```kiwi
import "string"

println string::islower("hello")    # prints: true
println string::islower("Hello")    # prints: false
println string::islower("hello 1")  # prints: true
```

### `isnumeric(_input)`

Checks if the string consists only of digit characters (0–9).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` if the string contains only digits. |

**Example**
```kiwi
import "string"

println string::isnumeric("12345")   # prints: true
println string::isnumeric("123.4")   # prints: false
println string::isnumeric("12 34")   # prints: false
```

### `isupper(_input)`

Checks if the string is entirely uppercase (no lowercase letters present).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` if the string has no lowercase letters. |

**Example**
```kiwi
import "string"

println string::isupper("HELLO")    # prints: true
println string::isupper("Hello")    # prints: false
println string::isupper("HELLO 1")  # prints: true
```

### `mirror(_input, _delimiter = "")`

Appends the reverse of the string to itself, optionally separated by a delimiter.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `string` | `_delimiter` | Separator between the original and its reverse. Defaults to `""`. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The mirrored string. |

**Example**
```kiwi
import "string"

println string::mirror("hello")         # prints: helloolleh
println string::mirror("kiwi", "|")     # prints: kiwi|iwik
println string::mirror("abc", " - ")    # prints: abc - cba
```

### `padstart(_input, _size, _padding = " ")`

Left-pads a string to `_size` characters.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_size` | The total width of the output string. |
| `string` | `_padding` | The padding character. Defaults to a single space ` `. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The left-padded string. |

**Example**
```kiwi
import "string"

println string::padstart("42", 6)        # prints: "    42"
println string::padstart("42", 6, "0")   # prints: "000042"
println string::padstart("hi", 6, "-")   # prints: "----hi"
```

### `padend(_input, _size, _padding = " ")`

Right-pads a string to `_size` characters.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_size` | The total width of the output string. |
| `string` | `_padding` | The padding character. Defaults to a single space ` `. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The right-padded string. |

**Example**
```kiwi
import "string"

println string::padend("hi", 6)        # prints: "hi    "
println string::padend("hi", 6, ".")   # prints: "hi...."
```

### `shuffle(_input)`

Returns a new string with the characters in a random order.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The shuffled string. |

**Example**
```kiwi
import "string"

s = string::shuffle("hello")
println s.size()           # prints: 5  (same length)
println s.chars().sort()   # prints: ["e", "h", "l", "l", "o"]  (same chars)
```

### `slug(_input)`

Generates a URL-friendly slug: lowercases, replaces spaces and special characters with hyphens, and strips leading/trailing hyphens.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The generated slug. |

**Example**
```kiwi
import "string"

println string::slug("Hello, World!")           # prints: hello-world
println string::slug("The Quick Brown Fox")     # prints: the-quick-brown-fox
println string::slug("  leading & trailing  ")  # prints: leading-trailing
```

### `tolower(_input)`

Converts all characters in the string to lowercase.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The string converted to lowercase. |

**Example**
```kiwi
import "string"

println string::tolower("HELLO World")   # prints: hello world
```

### `toupper(_input)`

Converts all characters in the string to uppercase.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The string converted to uppercase. |

**Example**
```kiwi
import "string"

println string::toupper("hello World")   # prints: HELLO WORLD
```

### `urldecode(_input)`

Decodes a percent-encoded URL string to plaintext.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The URL-encoded string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The decoded plaintext string. |

**Example**
```kiwi
import "string"

println string::urldecode("hello%20world")   # prints: hello world
println string::urldecode("foo%3Dbar")       # prints: foo=bar
```

### `urlencode(_input)`

Percent-encodes a string for safe use in a URL.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The string to encode. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The URL-encoded string. |

**Example**
```kiwi
import "string"

println string::urlencode("hello world")   # prints: hello+world
println string::urlencode("foo=bar&x=1")   # prints: foo%3Dbar%26x%3D1
```

### `capitalize(_input)`

Returns the string with the first character uppercased and the rest lowercased. Also available as `capitalise(_input)`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The capitalized string. |

**Example**
```kiwi
import "string"

println string::capitalize("hello world")   # prints: Hello world
println string::capitalize("HELLO")         # prints: Hello
println string::capitalise("kiwi")          # prints: Kiwi
```

### `titleize(_input)`

Returns the string with the first character of every word uppercased and the rest lowercased. Also available as `title(_input)`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The title-cased string. |

**Example**
```kiwi
import "string"

println string::titleize("the quick brown fox")   # prints: The Quick Brown Fox
println string::title("hello world")              # prints: Hello World
```

### `smart_titleize(s, keep_lowercase?)`

Title-cases a string while keeping common short words (articles, prepositions, conjunctions) lowercase — unless they are the first word.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `s` | The input string. |
| `list` | `keep_lowercase` | A list of words to keep lowercase (e.g. `["a", "an", "the", "of"]`). Defaults to a standard English set. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The smart title-cased string. |

**Example**
```kiwi
println string::smart_titleize("the lord of the rings")
# prints: The Lord of the Rings
```

### `dedent(text)`

Removes common leading whitespace from every line in a multi-line string. Useful for cleaning up indented heredoc-style strings.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `text` | The input string. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The dedented string. |

**Example**
```kiwi
s = "
  hello
  world
"
println string::dedent(s)
# prints:
# hello
# world
```

### `to_path(path)`

Converts a string to a `Path` instance from the `path` package.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | The path string. |

**Returns**

| Type | Description |
| :--- | :---|
| `Path` | A new `Path` instance. |

**Example**
```kiwi
p = string::to_path("/home/user/docs")
println p.is_dir()  # prints: true (if directory exists)
```