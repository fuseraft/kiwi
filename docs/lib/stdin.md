# `stdin`

The `stdin` package provides functions for reading from standard input in Kiwi. These functions allow both blocking and non-blocking reads, as well as convenient ways to consume all available input.

## Table of Contents

- [Package Functions](#package-functions)
  - [`read()`](#read)
  - [`readbytes()`](#readbytes)
  - [`readline()`](#readline)
  - [`lines()`](#lines)
  - [`empty()`](#empty)

## Package Functions

### `read()`
Reads all available input from stdin as a string until EOF or no more data is immediately available.

**Parameters**  
_None_

**Returns**  
| Type | Description |
| :--- | :--- |
| `string` | The input data as a string. |

---

### `readbytes()`
Reads all available input from stdin as raw bytes until EOF or no more data is immediately available.

**Parameters**  
_None_

**Returns**  
| Type | Description |
| :--- | :--- |
| `bytes` | The input data as bytes. |

---

### `readline()`
Reads a single line from stdin. The returned string does not include the trailing newline.

**Parameters**  
_None_

**Returns**  
| Type | Description |
| :--- | :--- |
| `string` | A single line of input. |

---

### `lines()`
Reads all complete lines currently available from stdin and returns them as a list of strings. Each string has its trailing newline removed.

**Parameters**  
_None_

**Returns**  
| Type | Description |
| :--- | :--- |
| `list` | A list of strings, each representing one available line. |

---

### `empty()`
Checks whether there is any data currently available on stdin.

**Parameters**  
_None_

**Returns**  
| Type | Description |
| :--- | :--- |
| `boolean` | `true` if no data is available, `false` otherwise. |