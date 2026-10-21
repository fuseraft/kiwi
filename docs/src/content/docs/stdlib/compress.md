---
title: compress
description: Compress and decompress data with DEFLATE, GZip, Brotli, Zstandard and ZIP.
---

The `compress` package compresses and decompresses data in memory. Every function accepts a `string` or `bytes` and returns `bytes`; call `.to_string()` on the result to get text back.

| Format | Compress | Decompress |
| :-- | :-- | :-- |
| DEFLATE | `deflate` | `inflate` |
| GZip | `gzip` | `gunzip` |
| Brotli | `brotli` | `unbrotli` |
| Zstandard | `zstd` | `unzstd` |
| ZIP archive | `zip_create` | `zip_extract` |

## Round trips

```kiwi
text = "Kiwi is a scripting language. " * 50

packed = compress::gzip(text)
println packed.size() < text.size()        # true
println compress::gunzip(packed).to_string() == text   # true

smaller = compress::zstd(text, 10)          # Zstandard, compression level 10
println compress::unzstd(smaller).to_string() == text  # true
```

To write compressed data to disk, use [`fio::writebytes`](/kiwi/stdlib/fio/):

```kiwi nocheck
fio::writebytes("log.txt.gz", compress::gzip(fio::read("log.txt")))
```

## ZIP archives

`zip_create` takes a list of `{name, content}` hashmaps and returns the archive as bytes. `zip_extract` does the reverse.

```kiwi
archive = compress::zip_create([
  {name: "hello.txt", content: "Hello from Kiwi!"},
  {name: "data.csv", content: "1,2,3\n4,5,6"}
])

for entry in compress::zip_extract(archive) do
  println "${entry["name"]}: ${entry["content"].size()} bytes"
end
```

<!-- api: compress -->
