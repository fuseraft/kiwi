# `crypto`

The `crypto` package contains functionality for generating hash strings.

---

## Package Functions

### `md5_hash(input = "")`
Generates an MD5 hash string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | The input string.|

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | An MD5 hash. |

**Example**

```kiwi
import "crypto"

var hash = crypto::md5_hash("hello")
println hash
# 5d41402abc4b2a76b9719d911017c592
```

### `sha224_hash(input = "")`
Generates a SHA-224 hash string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | The input string.|

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A SHA-224 hash. |

**Example**

```kiwi
import "crypto"

var hash = crypto::sha224_hash("hello")
println hash
# ea09ae9cc6768c50fcee903ed054556e5bfc8347907f12598aa24193
```

### `sha256_hash(input = "")`
Generates a SHA-256 hash string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | The input string.|

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A SHA-256 hash. |

**Example**

```kiwi
import "crypto"

var hash = crypto::sha256_hash("hello")
println hash
# 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824

# Hashing user input or file content
var password = "s3cr3t!"
var digest = crypto::sha256_hash(password)
println "SHA-256: " + digest
```
