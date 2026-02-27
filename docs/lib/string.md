# `string`

The `string` package contains specialized functions for strings.

## Table of Contents

- [Package Functions](#package-functions)
  - [`base64decode(_input)`](#base64decode_input)
  - [`base64encode(_input)`](#base64encode_input)
  - [`capitalize(_input)`](#capitalize_input)
  - [`center(_input, _size, _padding = " ")`](#center_input-_size-_padding)
  - [`contains_any(str, matches = [])`](#contains_anystr-matches--)
  - [`dedent(text)`](#dedenttext)
  - [`echo(_input, _count)`](#echo_input-_count)
  - [`interweave(_input1, _input2)`](#interweave_input1-_input2)
  - [`isalpha(_input)`](#isalpha_input)
  - [`isalphanumeric(_input)`](#isalphanumeric_input)
  - [`isnumeric(_input)`](#isnumeric_input)
  - [`islower(_input)`](#islower_input)
  - [`isupper(_input)`](#isupper_input)
  - [`mirror(_input, _delimiter = "")`](#mirror_input-_delimiter)
  - [`padstart(_input, _size, _padding = " ")`](#padstart_input-_size-_padding)
  - [`padend(_input, _size, _padding = " ")`](#padend_input-_size-_padding)
  - [`shuffle(_input)`](#shuffle_input)
  - [`slug(_input)`](#slug_input)
  - [`smart_titleize(s, keep_lowercase?)`](#smart_titleizes-keep_lowercase)
  - [`titleize(_input)`](#titleize_input)
  - [`to_path(path)`](#to_pathpath)
  - [`tolower(_input)`](#tolower_input)
  - [`toupper(_input)`](#toupper_input)
  - [`urldecode(_input)`](#urldecode_input)
  - [`urlencode(_input)`](#urlencode_input)

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

### `center(_input, _size, _padding = " ")`

Pads both sides of a string, centering it.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_size` | The max length of the output string. |
| `string` | `_padding` | The padding content. Defaults to a single space ` `. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The centered string. |

### `contains_any(str, matches = [])`

Returns `true` if `str` contains any values in `matches`.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `str` | The input string. |
| `list` | `matches` | The strings to match against. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | True if found. |

### `echo(_input, _count)`

Repeats each character in the string a specified number of times.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_count` | The number of times each character is repeated. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The string with characters echoed. |

### `interweave(_input1, _input2)`

Interweaves characters from two strings, alternating one character from each.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input1` | The first input string. |
| `string` | `_input2` | The second input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The interwoven string. |

### `isalpha(_input)`

Checks if the string consists only of alphabetic characters.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | Returns `true` if the string is alphabetic, otherwise `false`. |

### `isalphanumeric(_input)`

Checks if the string consists only of alphanumeric characters

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | Returns `true` if the string is alphanumeric, otherwise `false`. |

### `islower(_input)`

Checks if the string is entirely in lowercase.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | Returns `true` if the string is all lowercase, otherwise `false`. |

### `isnumeric(_input)`

Checks if the string consists only of numeric characters.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | Returns `true` if the string is numeric, otherwise `false`. |

### `isupper(_input)`

Checks if the string is entirely in uppercase.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | Returns `true` if the string is all uppercase, otherwise `false`. |

### `mirror(_input, _delimiter = "")`

Creates a mirrored version of the input string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `string` | `_delimiter` | The delimiter to separate the original string from its mirror. Defaults to an empty string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The mirrored string. |

### `padstart(_input, _size, _padding = " ")`

Pads the left side of a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_size` | The max length of the output string. |
| `string` | `_padding` | The padding content. Defaults to a single space ` `. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The padded string. |

### `padend(_input, _size, _padding = " ")`

Pads the right side of a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |
| `integer` | `_size` | The max length of the output string. |
| `string` | `_padding` | The padding content. Defaults to a single space ` `. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The padded string. |

### `shuffle(_input)`

Randomly shuffles the characters in a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The shuffled string. |

### `slug(_input)`

Generates a URL-friendly slug from a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The generated slug. |

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

### `urldecode(_input)`

Decodes a URL encoded string to plaintext.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The plaintext string. |

### `urlencode(_input)`

Encodes a string as a URL encoded string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The URL encoded string. |

### `capitalize(_input)`

Returns a copy of the string with the first character uppercased and the rest lowercased. Also available as `capitalise(_input)`.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The capitalized string. |

### `titleize(_input)`

Returns a copy of the string with the first character of every word uppercased and the rest lowercased. Also available as `title(_input)`.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The title-cased string. |

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