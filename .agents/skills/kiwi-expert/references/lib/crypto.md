# crypto

> Hash strings, bytes and files with MD5 and the SHA family, and compute HMACs.

The `crypto` package computes message digests and HMACs. Each function takes a string, which it hashes as UTF-8, or bytes, which it hashes as they are, and returns a lowercase hexadecimal string.

```kiwi
println crypto::sha256_hash("kiwi")
# 1a5afeda973d776e31d1d7266f184468f84d99bed311d88d3dcb67015934f9f9

println crypto::md5_hash("kiwi")
# de5949721e6352f01dfef317c3e898a8

println crypto::hmac_sha256("secret-key", "message to sign")
```

| Function | Algorithm |
| :-- | :-- |
| `md5_hash` | MD5 |
| `sha1_hash` | SHA-1 |
| `sha224_hash`, `sha256_hash`, `sha384_hash`, `sha512_hash` | SHA-2 |
| `hmac_md5`, `hmac_sha256`, `hmac_sha512` | HMAC with that hash |
| `hmac(algorithm, key, input)` | HMAC with a named algorithm |
| `file_hash(path, algorithm)` | A file's contents, with MD5, SHA-1, SHA-256 (the default), SHA-384 or SHA-512 |

`file_hash` reads the file in chunks, so it can hash files larger than memory, such as a download you want to check against a published checksum:

```kiwi
println crypto::file_hash("kiwi-linux-x64.tar.gz")             # SHA-256
println crypto::file_hash("kiwi-linux-x64.tar.gz", "SHA512")
```

`file_hash` is unavailable in [safe mode](../tools/cli.md#safe-mode).

MD5 and SHA-1 are fine for checksums and cache keys but are no longer safe against deliberate tampering; use SHA-256 or better for anything security-related. These functions hash data; they are not suitable for storing passwords, which need a slow, salted algorithm.

For Base64 and hex encoding, see [`encode`](encode.md).

## Functions

### `crypto::md5_hash`

```kiwi
crypto::md5_hash(input = "")
```

Computes the MD5 hex digest of a string or bytes. A string is hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string\|bytes` | `""` | The input to hash (default `""`). |

**Returns** `string` — A 32-character lowercase hexadecimal MD5 digest.

### `crypto::sha224_hash`

```kiwi
crypto::sha224_hash(input = "")
```

Computes the SHA-224 hex digest of a string or bytes. A string is hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string\|bytes` | `""` | The input to hash (default `""`). |

**Returns** `string` — A 56-character lowercase hexadecimal SHA-224 digest.

### `crypto::sha256_hash`

```kiwi
crypto::sha256_hash(input = "")
```

Computes the SHA-256 hex digest of a string or bytes. A string is hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string\|bytes` | `""` | The input to hash (default `""`). |

**Returns** `string` — A 64-character lowercase hexadecimal SHA-256 digest.

### `crypto::sha1_hash`

```kiwi
crypto::sha1_hash(input = "")
```

Computes the SHA-1 hex digest of a string or bytes. A string is hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string\|bytes` | `""` | The input to hash (default `""`). |

**Returns** `string` — A 40-character lowercase hexadecimal SHA-1 digest.

### `crypto::sha384_hash`

```kiwi
crypto::sha384_hash(input = "")
```

Computes the SHA-384 hex digest of a string or bytes. A string is hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string\|bytes` | `""` | The input to hash (default `""`). |

**Returns** `string` — A 96-character lowercase hexadecimal SHA-384 digest.

### `crypto::sha512_hash`

```kiwi
crypto::sha512_hash(input = "")
```

Computes the SHA-512 hex digest of a string or bytes. A string is hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string\|bytes` | `""` | The input to hash (default `""`). |

**Returns** `string` — A 128-character lowercase hexadecimal SHA-512 digest.

### `crypto::file_hash`

```kiwi
crypto::file_hash(path, algorithm = "SHA256")
```

Computes the hex digest of a file's contents. The file is read in chunks, so it can be larger than memory. Unavailable in safe mode.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | The file to hash. |
| `algorithm` | `string` | `"SHA256"` | One of "MD5", "SHA1", "SHA256", "SHA384" or "SHA512" (default "SHA256"). |

**Returns** `string` — Lowercase hexadecimal digest.

**Throws** `FileReadError` — If the file can't be read.

### `crypto::hmac`

```kiwi
crypto::hmac(algorithm, key, input)
```

Computes an HMAC hex digest. Strings are hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `algorithm` | `string` |  | One of "MD5", "SHA256", "SHA512". |
| `key` | `string\|bytes` |  | The HMAC key. |
| `input` | `string\|bytes` |  | The input. |

**Returns** `string` — Lowercase hexadecimal HMAC digest.

### `crypto::hmac_sha256`

```kiwi
crypto::hmac_sha256(key, input)
```

Computes HMAC-SHA256. Strings are hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string\|bytes` |  | The HMAC key. |
| `input` | `string\|bytes` |  | The input. |

**Returns** `string` — Lowercase hexadecimal digest.

### `crypto::hmac_sha512`

```kiwi
crypto::hmac_sha512(key, input)
```

Computes HMAC-SHA512. Strings are hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string\|bytes` |  | The HMAC key. |
| `input` | `string\|bytes` |  | The input. |

**Returns** `string` — Lowercase hexadecimal digest.

### `crypto::hmac_md5`

```kiwi
crypto::hmac_md5(key, input)
```

Computes HMAC-MD5. Strings are hashed as UTF-8.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string\|bytes` |  | The HMAC key. |
| `input` | `string\|bytes` |  | The input. |

**Returns** `string` — Lowercase hexadecimal digest.

## `MD5` struct

A pure-Kiwi implementation of the MD5 message-digest algorithm.

### `MD5.hexdigest`

```kiwi
static MD5.hexdigest(input)
```

Computes the MD5 hex digest of a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The input string to hash. |

**Returns** `string` — A 32-character lowercase hexadecimal MD5 digest.

### `MD5.left_rotate`

```kiwi
static MD5.left_rotate(value, count)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `value` | `any` |  |  |
| `count` | `any` |  |  |

### `MD5.int_to_bytes`

```kiwi
static MD5.int_to_bytes(num)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `num` | `any` |  |  |

## `SHA` struct

A SHA-2 implementation based on the implementation in PyPy https://foss.heptapod.net/pypy/pypy/-/blob/branch/default/lib_pypy/_sha256.py

### `SHA.BLOCKSIZE`

```kiwi
static SHA.BLOCKSIZE()
```

### `SHA.DIGESTSIZE`

```kiwi
static SHA.DIGESTSIZE()
```

### `SHA.new_shaobject`

```kiwi
static SHA.new_shaobject()
```

### `SHA.Ch`

```kiwi
static SHA.Ch(x, y, z)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |
| `y` | `any` |  |  |
| `z` | `any` |  |  |

### `SHA.Maj`

```kiwi
static SHA.Maj(x, y, z)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |
| `y` | `any` |  |  |
| `z` | `any` |  |  |

### `SHA.S`

```kiwi
static SHA.S(x, n)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |
| `n` | `any` |  |  |

### `SHA.R`

```kiwi
static SHA.R(x, n)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |
| `n` | `any` |  |  |

### `SHA.Sigma0`

```kiwi
static SHA.Sigma0(x)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |

### `SHA.Sigma1`

```kiwi
static SHA.Sigma1(x)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |

### `SHA.Gamma0`

```kiwi
static SHA.Gamma0(x)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |

### `SHA.Gamma1`

```kiwi
static SHA.Gamma1(x)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `any` |  |  |

### `SHA.RND`

```kiwi
static SHA.RND(a, b, c, d, e, f, g, h, i, ki, W)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `a` | `any` |  |  |
| `b` | `any` |  |  |
| `c` | `any` |  |  |
| `d` | `any` |  |  |
| `e` | `any` |  |  |
| `f` | `any` |  |  |
| `g` | `any` |  |  |
| `h` | `any` |  |  |
| `i` | `any` |  |  |
| `ki` | `any` |  |  |
| `W` | `any` |  |  |

### `SHA.transform`

```kiwi
static SHA.transform(sha_info)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sha_info` | `any` |  |  |

### `SHA.sha_init`

```kiwi
static SHA.sha_init()
```

### `SHA.sha224_init`

```kiwi
static SHA.sha224_init()
```

### `SHA.getbuf`

```kiwi
static SHA.getbuf(s)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `any` |  |  |

### `SHA.sha_update`

```kiwi
static SHA.sha_update(sha_info, buffer)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sha_info` | `any` |  |  |
| `buffer` | `any` |  |  |

### `SHA.sha_final`

```kiwi
static SHA.sha_final(sha_info)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sha_info` | `any` |  |  |

## `SHA256` struct

A pure-Kiwi SHA-256 hasher supporting incremental updates.

### `SHA256.new`

```kiwi
SHA256.new(s = null)
```

Creates a new SHA-256 hasher, optionally hashing an initial string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string\|none` | `null` | An optional initial input string (default `null`). |

### `.update`

```kiwi
update(s)
```

Feeds additional data into the hasher.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string` |  | The data to hash. |

### `.digest`

```kiwi
digest()
```

Returns the raw digest as a list of bytes.

**Returns** `list` — A list of integers representing the digest bytes.

### `.hexdigest`

```kiwi
hexdigest()
```

Returns the hex-encoded digest string.

**Returns** `string` — A 64-character lowercase hexadecimal SHA-256 digest.

### `.copy`

```kiwi
copy()
```

Returns a copy of this hasher with the same internal state.

**Returns** `SHA256` — A new independent SHA256 instance.

## `SHA224` struct

Inherits from `SHA256`.

A pure-Kiwi SHA-224 hasher. Inherits from `SHA256` with a 224-bit digest.

### `SHA224.new`

```kiwi
SHA224.new(s = null)
```

Creates a new SHA-224 hasher, optionally hashing an initial string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string\|none` | `null` | An optional initial input string (default `null`). |

### `.update`

```kiwi
update(s)
```

Feeds additional data into the hasher.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string` |  | The data to hash. |

### `.digest`

```kiwi
digest()
```

Returns the raw digest as a list of bytes.

**Returns** `list` — A list of integers representing the digest bytes.

### `.hexdigest`

```kiwi
hexdigest()
```

Returns the hex-encoded digest string.

**Returns** `string` — A 56-character lowercase hexadecimal SHA-224 digest.

### `.copy`

```kiwi
copy()
```

Returns a copy of this hasher with the same internal state.

**Returns** `SHA224` — A new independent SHA224 instance.
