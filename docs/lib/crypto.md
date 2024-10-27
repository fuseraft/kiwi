# `crypto`

The `crypto` package contains functionality for generating hash strings.

## Table of Contents

- [Package Functions](#package-functions)
  - [`md5_hash(input)`](#md5_hashinput--)
  - [`sha224_hash(input)`](#sha224_hashinput--)
  - [`sha256_hash(input)`](#sha256_hashinput--)

## Package Functions

### `md5_hash(input = "")`
Generates an MD5 hash string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `input` | The input string.|

**Returns**
| Type | Description |
| :--- | :--- |
| `String` | An MD5 hash. |

### `sha224_hash(input = "")`
Generates a SHA-224 hash string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `input` | The input string.|

**Returns**
| Type | Description |
| :--- | :--- |
| `String` | A SHA-224 hash. |

### `sha256_hash(input = "")`
Generates a SHA-56 hash string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `input` | The input string.|

**Returns**
| Type | Description |
| :--- | :--- |
| `String` | A SHA-256 hash. |
