# `compress`

The `compress` package provides a clean, idiomatic interface to Kiwi's built-in compression and archive functions.

It wraps the low-level builtins (`__compress_*__`, `__decompress_*__`, `__zip_*__`) with shorter/human-friendly names and sensible defaults, making compression and decompression tasks feel natural in Kiwi.

## Table of Contents

- [Package Functions](#package-functions)
  - [`deflate(data)`](#deflatedata)
  - [`inflate(data)`](#inflatedata)
  - [`gzip(data)`](#gzipdata)
  - [`gunzip(data)`](#gunzipdata)
  - [`brotli(data)`](#brotlidata)
  - [`unbrotli(data)`](#unbrotlidata)
  - [`zstd(data, level=3)`](#zstddata-level3)
  - [`unzstd(data)`](#unzstddata)
  - [`zip_create(entries, level=1)`](#zip_createentries-level1)
  - [`zip_extract(data)`](#zip_extractdata)

## Package Functions

All functions accept either a `string` or `bytes` as input data (strings are automatically UTF-8 encoded to bytes).  

They return `bytes` containing the compressed / decompressed / archived data.

### `deflate(data)`
Compresses data using the raw **DEFLATE** algorithm (zlib without header/trailer).

**Parameters**  
| Type          | Name   | Description                          | Default |
|---------------|--------|--------------------------------------|---------|
| `string` or `bytes` | `data` | Data to compress                     | —       |

**Returns**  
| Type   | Description                     |
|--------|---------------------------------|
| `bytes` | Compressed data (DEFLATE format) |

---

### `inflate(data)`
Decompresses raw **DEFLATE** data.

**Parameters**  
Same as `deflate`.

**Returns**  
| Type   | Description                       |
|--------|-----------------------------------|
| `bytes` | Decompressed original data        |

---

### `gzip(data)`
Compresses data using **GZip** (DEFLATE + gzip header + CRC).

**Parameters**  
Same as `deflate`.

**Returns**  
| Type   | Description                     |
|--------|---------------------------------|
| `bytes` | GZip-compressed data            |

---

### `gunzip(data)`
Decompresses **GZip** data.

**Parameters**  
Same as `deflate`.

**Returns**  
| Type   | Description                       |
|--------|-----------------------------------|
| `bytes` | Original decompressed data        |

---

### `brotli(data)`
Compresses data using **Brotli**.

**Parameters**  
Same as `deflate`.

**Returns**  
| Type   | Description                       |
|--------|-----------------------------------|
| `bytes` | Brotli-compressed data            |

---

### `unbrotli(data)`
Decompresses **Brotli** data.

**Parameters**  
Same as `deflate`.

**Returns**  
| Type   | Description                       |
|--------|-----------------------------------|
| `bytes` | Original decompressed data        |

---

### `zstd(data, level=3)`
Compresses data using **Zstandard** (modern, very fast + good ratio).

**Parameters**  
| Type          | Name     | Description                                      | Default |
|---------------|----------|--------------------------------------------------|---------|
| `string` or `bytes` | `data`   | Data to compress                                 | —       |
| `integer`     | `level`  | Compression level (1–22; higher = better ratio, slower) | `3`     |

**Returns**  
| Type   | Description                       |
|--------|-----------------------------------|
| `bytes` | Zstandard-compressed data         |

**Note**: Levels 1–22 are standard; higher levels give better compression at the cost of speed and memory.

---

### `unzstd(data)`
Decompresses **Zstandard** data.

**Parameters**  
Same as `deflate` (no level needed for decompression).

**Returns**  
| Type   | Description                       |
|--------|-----------------------------------|
| `bytes` | Original decompressed data        |

---

### `zip_create(entries, level=1)`
Creates a **ZIP** archive from a list of file entries.

**Parameters**  
| Type                | Name      | Description                                                                 | Default       |
|---------------------|-----------|-----------------------------------------------------------------------------|---------------|
| `list<hashmap>`     | `entries` | List of maps: each must have keys `"name"` (string) and `"content"` (string/bytes) | —             |
| `integer`           | `level`   | Compression level for Deflate in ZIP (0 = store, 1 = optimal, 2 = fastest)  | `1` (optimal) |

**Example entry**
```kiwi
{name: "readme.txt", content: "Hello, Kiwi!"}
```

**Returns**  
| Type   | Description                     |
|--------|---------------------------------|
| `bytes` | ZIP archive data                |

**Throws**  
If any entry is missing `"name"` or `"content"`, or if types are invalid.

---

### `zip_extract(data)`
Extracts all files from a **ZIP** archive.

**Parameters**  
| Type          | Name   | Description                     | Default |
|---------------|--------|---------------------------------|---------|
| `bytes`       | `data` | ZIP archive bytes               | —       |

**Returns**  
| Type              | Description                                      |
|-------------------|--------------------------------------------------|
| `list<hashmap>`   | List of `{name: string, content: bytes}` entries |

**Note**: File order is not guaranteed (ZIP does not preserve order).

---

## Usage Examples

```kiwi
# Simple round-trip
text = "Kiwi is a fast, modern scripting language " * 50
packed = compress::zstd(text, 10)
unpacked = compress::unzstd(packed)
tester::assert_eq(text, unpacked.to_string())

# ZIP archive
files = [
  { name: "hello.txt", content: "Hello from Kiwi!" },
  { name: "data.csv",  content: "1,2,3\n4,5,6" }
]
archive = compress::zip_create(files)
extracted = compress::zip_extract(archive)

println("Extracted files:")
for f in extracted do
  println("  " + f.name + " (" + f.content.size() + " bytes)")
end
```