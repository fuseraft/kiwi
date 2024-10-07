# `string`

The `string` package contains specialized functions for strings.

## Table of Contents

- [Package Functions](#package-functions)
  - [`base64decode(_input)`](#base64decode_input)
  - [`base64encode(_input)`](#base64encode_input)
  - [`center(_input, _size, _padding = " ")`](#center_input-_size-_padding)
  - [`contains_any(str, matches = [])`](#contains_anystr-matches--)
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
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The plaintext string. |

### `base64encode(_input)`

Encodes a string as a base64 string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The base64 encoded string. |

### `center(_input, _size, _padding = " ")`

Pads both sides of a string, centering it.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |
| `Integer` | `_size` | The max length of the output string. |
| `String` | `_padding` | The padding content. Defaults to a single space ` `. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The centered string. |

### contains_any(str, matches = [])

Returns `true` if `str` contains any values in `matches`.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `str` | The input string. |
| `List` | `matches` | The strings to match against. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | True if found. |

### `echo(_input, _count)`

Repeats each character in the string a specified number of times.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |
| `Integer` | `_count` | The number of times each character is repeated. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The string with characters echoed. |

### `interweave(_input1, _input2)`

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input1` | The first input string. |
| `String` | `_input2` | The second input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The interwoven string. |

### `isalpha(_input)`

Checks if the string consists only of alphabetic characters.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Returns `true` if the string is alphabetic, otherwise `false`. |

### `isalphanumeric(_input)`

Checks if the string consists only of alphanumeric characters

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Returns `true` if the string is alphanumeric, otherwise `false`. |

### `islower(_input)`

Checks if the string is entirely in lowercase.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Returns `true` if the string is all lowercase, otherwise `false`. |

### `isnumeric(_input)`

Checks if the string consists only of numeric characters.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Returns `true` if the string is numeric, otherwise `false`. |

### `isupper(_input)`

Checks if the string is entirely in uppercase.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Returns `true` if the string is all uppercase, otherwise `false`. |

### `mirror(_input, _delimiter = "")`

Creates a mirrored version of the input string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |
| `String` | `_delimiter` | The delimiter to separate the original string from its mirror. Defaults to an empty string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The mirrored string. |

### `padstart(_input, _size, _padding = " ")`

Pads the left side of a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |
| `Integer` | `_size` | The max length of the output string. |
| `String` | `_padding` | The padding content. Defaults to a single space ` `. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The padded string. |

### `padend(_input, _size, _padding = " ")`

Pads the right side of a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |
| `Integer` | `_size` | The max length of the output string. |
| `String` | `_padding` | The padding content. Defaults to a single space ` `. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The padded string. |

### `shuffle(_input)`

Randomly shuffles the characters in a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The shuffled string. |

### `slug(_input)`

Generates a URL-friendly slug from a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The generated slug. |

### `tolower(_input)`

Converts all characters in the string to lowercase.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The string converted to lowercase. |

### `toupper(_input)`

Converts all characters in the string to uppercase.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The string converted to uppercase. |

### `urldecode(_input)`

Decodes a URL encoded string to plaintext.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The plaintext string. |

### `urlencode(_input)`

Encodes a string as a URL encoded string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_input` | The input string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The URL encoded string. |