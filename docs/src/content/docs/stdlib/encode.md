---
title: encode
description: Base64, hexadecimal and URL encoding.
---

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

Strings also have `.base64encode()`, `.base64decode()`, `.urlencode()` and `.urldecode()` methods from the [`string`](/kiwi/stdlib/string/) package. For hashing, see [`crypto`](/kiwi/stdlib/crypto/).

<!-- api: encode -->
