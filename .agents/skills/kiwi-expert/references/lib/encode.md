# encode

> Base64, hexadecimal and URL encoding.

The `encode` package converts data to and from text encodings.

```kiwi
println encode::base64_encode("kiwi")                    # a2l3aQ==
println encode::base64_decode("a2l3aQ==").to_string()    # kiwi

println encode::hex_encode("kiwi")                       # 6b697769
println encode::hex_decode("6b697769").to_string()       # kiwi

println encode::url_encode("a b&c")                      # a+b%26c
println encode::url_decode("a+b%26c")                    # a b&c
```

The encoders accept a string or bytes. `base64_decode` and `hex_decode` return bytes, since the decoded data may not be text; call `.to_string()` to decode it as UTF-8.

Strings also have `.base64encode()`, `.base64decode()`, `.urlencode()` and `.urldecode()` methods from the [`string`](string.md) package. For hashing, see [`crypto`](crypto.md).

## Functions

### `encode::base64_encode`

```kiwi
encode::base64_encode(data)
```

Encodes a string or bytes to Base64.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `string\|bytes\|list` |  | The data to encode. |

**Returns** `string` — Base64-encoded string.

### `encode::base64_decode`

```kiwi
encode::base64_decode(data)
```

Decodes a Base64 string to bytes.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `string` |  | Base64-encoded string. |

**Returns** `bytes` — Decoded bytes.

### `encode::hex_encode`

```kiwi
encode::hex_encode(data)
```

Encodes a string or bytes to lowercase hex.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `string\|bytes\|list` |  | The data to encode. |

**Returns** `string` — Hex-encoded string.

### `encode::hex_decode`

```kiwi
encode::hex_decode(hex_str)
```

Decodes a hex string to bytes.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `hex_str` | `string` |  | Hex-encoded string (spaces and dashes ignored). |

**Returns** `bytes` — Decoded bytes.

### `encode::url_encode`

```kiwi
encode::url_encode(str)
```

URL-encodes a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `str` | `string` |  | String to encode. |

**Returns** `string` — URL-encoded string.

### `encode::url_decode`

```kiwi
encode::url_decode(str)
```

URL-decodes a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `str` | `string` |  | URL-encoded string. |

**Returns** `string` — Decoded string.
