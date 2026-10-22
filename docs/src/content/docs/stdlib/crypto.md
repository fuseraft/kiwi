---
title: crypto
description: Hash strings, bytes and files with MD5 and the SHA family, and compute HMACs.
---

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

```kiwi nocheck
println crypto::file_hash("kiwi-linux-x64.tar.gz")             # SHA-256
println crypto::file_hash("kiwi-linux-x64.tar.gz", "SHA512")
```

`file_hash` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode).

MD5 and SHA-1 are fine for checksums and cache keys but are no longer safe against deliberate tampering; use SHA-256 or better for anything security-related. These functions hash data; they are not suitable for storing passwords, which need a slow, salted algorithm.

For Base64 and hex encoding, see [`encode`](/kiwi/stdlib/encode/).

<!-- api: crypto -->
