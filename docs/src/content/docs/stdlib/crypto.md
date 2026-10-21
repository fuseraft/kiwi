---
title: crypto
description: Hash strings with MD5 and the SHA family, and compute HMACs.
---

The `crypto` package computes message digests and HMACs. Each function returns a lowercase hexadecimal string.

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

MD5 and SHA-1 are fine for checksums and cache keys but are no longer safe against deliberate tampering; use SHA-256 or better for anything security-related. These functions hash data; they are not suitable for storing passwords, which need a slow, salted algorithm.

For Base64 and hex encoding, see [`encode`](/kiwi/stdlib/encode/).

<!-- api: crypto -->
