# `encode`

The `encode` package provides encoding and decoding utilities for Base64, hexadecimal, and URL formats.

---

## Package Functions

### `base64_encode(data)`

Encodes a string, bytes, or list to a Base64 string.

**Parameters**

| Type                    | Name   | Description          |
|-------------------------|--------|----------------------|
| `string\|bytes\|list`   | `data` | The data to encode   |

**Returns**

| Type     | Description             |
|----------|-------------------------|
| `string` | Base64-encoded string   |

---

### `base64_decode(data)`

Decodes a Base64 string to bytes.

**Parameters**

| Type     | Name   | Description              |
|----------|--------|--------------------------|
| `string` | `data` | Base64-encoded string    |

**Returns**

| Type    | Description     |
|---------|-----------------|
| `bytes` | Decoded bytes   |

---

### `hex_encode(data)`

Encodes a string, bytes, or list to a lowercase hexadecimal string.

**Parameters**

| Type                  | Name   | Description        |
|-----------------------|--------|--------------------|
| `string\|bytes\|list` | `data` | The data to encode |

**Returns**

| Type     | Description              |
|----------|--------------------------|
| `string` | Lowercase hex string     |

---

### `hex_decode(hex_str)`

Decodes a hexadecimal string to bytes. Spaces and dashes in the input are ignored.

**Parameters**

| Type     | Name      | Description                                  |
|----------|-----------|----------------------------------------------|
| `string` | `hex_str` | Hex-encoded string (spaces/dashes ignored)   |

**Returns**

| Type    | Description   |
|---------|---------------|
| `bytes` | Decoded bytes |

---

### `url_encode(str)`

Percent-encodes a string for use in a URL.

**Parameters**

| Type     | Name  | Description          |
|----------|-------|----------------------|
| `string` | `str` | String to encode     |

**Returns**

| Type     | Description          |
|----------|----------------------|
| `string` | URL-encoded string   |

---

### `url_decode(str)`

Decodes a percent-encoded URL string.

**Parameters**

| Type     | Name  | Description              |
|----------|-------|--------------------------|
| `string` | `str` | URL-encoded string       |

**Returns**

| Type     | Description        |
|----------|--------------------|
| `string` | Decoded string     |

---

## Examples

```kiwi
import "encode"

# Base64
encoded = encode::base64_encode("Hello, Kiwi!")
println encoded                          # SGVsbG8sIEtpd2kh

decoded_bytes = encode::base64_decode(encoded)
println decoded_bytes.to_string()        # Hello, Kiwi!

# Hex
hex = encode::hex_encode("abc")
println hex                              # 616263

raw = encode::hex_decode("61 62 63")     # spaces ignored
println raw.to_string()                  # abc

# URL
query = encode::url_encode("hello world & more")
println query                            # hello+world+%26+more

println encode::url_decode(query)        # hello world & more
```
