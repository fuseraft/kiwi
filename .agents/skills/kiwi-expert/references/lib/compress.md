# compress

> Compress and decompress data with DEFLATE, GZip, Brotli, Zstandard and ZIP.

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

To write compressed data to disk, use [`fio::writebytes`](fio.md):

```kiwi
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

## Functions

### `compress::deflate`

```kiwi
compress::deflate(data)
```

Compresses data using the Deflate algorithm.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The data to compress. |

**Returns** `bytes` — The compressed data.

### `compress::inflate`

```kiwi
compress::inflate(data)
```

Decompresses Deflate-compressed data.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `bytes` |  | The compressed data. |

**Returns** `any` — The decompressed data.

### `compress::gzip`

```kiwi
compress::gzip(data)
```

Compresses data using the gzip algorithm.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The data to compress. |

**Returns** `bytes` — The gzip-compressed data.

### `compress::gunzip`

```kiwi
compress::gunzip(data)
```

Decompresses gzip-compressed data.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `bytes` |  | The compressed data. |

**Returns** `any` — The decompressed data.

### `compress::brotli`

```kiwi
compress::brotli(data)
```

Compresses data using the Brotli algorithm.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The data to compress. |

**Returns** `bytes` — The Brotli-compressed data.

### `compress::unbrotli`

```kiwi
compress::unbrotli(data)
```

Decompresses Brotli-compressed data.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `bytes` |  | The compressed data. |

**Returns** `any` — The decompressed data.

### `compress::zstd`

```kiwi
compress::zstd(data, level = 3)
```

Compresses data using the Zstandard (zstd) algorithm.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The data to compress. |
| `level` | `integer` | `3` | Compression level (default 3). |

**Returns** `bytes` — The zstd-compressed data.

### `compress::unzstd`

```kiwi
compress::unzstd(data)
```

Decompresses Zstandard-compressed data.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `bytes` |  | The compressed data. |

**Returns** `any` — The decompressed data.

### `compress::zip_create`

```kiwi
compress::zip_create(entries, level = 1)
```

Creates a ZIP archive from a list of entries.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `entries` | `list` |  | A list of file entries to include in the archive. |
| `level` | `integer` | `1` | Compression level (default 1). |

**Returns** `bytes` — The ZIP archive as bytes.

### `compress::zip_extract`

```kiwi
compress::zip_extract(data)
```

Extracts entries from a ZIP archive.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `bytes` |  | The ZIP archive as bytes. |

**Returns** `list` — A list of extracted entries.
