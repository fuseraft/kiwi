# protobuf

> Encode and decode Protocol Buffers (proto3) with schemas written as hashmaps.

The `protobuf` package reads and writes the [Protocol Buffers](https://protobuf.dev/) proto3 binary format. There is no `.proto` compiler step: a schema is a hashmap, and the output is compatible with any other protobuf implementation.

```kiwi
schema = {
  1: {name: "id", type: "int32"},
  2: {name: "score", type: "double"},
  3: {name: "tags", type: "string", repeated: true},
  4: {name: "owner", type: "message", schema: {
    1: {name: "name", type: "string"}
  }}
}

message = {id: 7, score: 9.81, tags: ["dev", "kiwi"], owner: {name: "Ada"}}

encoded = protobuf::encode(schema, message)
println encoded.size()                     # 29 bytes
decoded = protobuf::decode(schema, encoded)
println decoded["owner"]["name"]           # Ada
println decoded["tags"]                    # ["dev", "kiwi"]
```

`encode_file(schema, data, path)` and `decode_file(schema, path)` read and write files directly.

## Schemas

A schema maps each **field number** to a descriptor:

| Key | Meaning |
| :-- | :-- |
| `name` | The key used in the data hashmap. |
| `type` | The wire type; see below. |
| `repeated` | `true` for a list field. |
| `schema` | The nested schema, for `type: "message"`. |

`protobuf::field(name, type, repeated, schema)` builds a descriptor if you prefer a function call.

### Field types

| Type | Encoding |
| :-- | :-- |
| `int32`, `int64`, `uint32`, `uint64`, `bool`, `enum` | Varint |
| `sint32`, `sint64` | Zigzag varint, compact for negative numbers |
| `float`, `fixed32`, `sfixed32` | 32-bit |
| `double`, `fixed64`, `sfixed64` | 64-bit |
| `string`, `bytes` | Length-delimited |
| `message` | Length-delimited nested message; needs `schema` |

Field numbers must match the ones used by whatever produces or consumes the data.

## Functions

### `protobuf::encode`

```kiwi
protobuf::encode(schema: hashmap, data: hashmap): bytes
```

Encodes a Kiwi hashmap into Protocol Buffer binary format.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `hashmap` |  | Field descriptor hashmap keyed by field number. |
| `data` | `hashmap` |  | Data to encode, keyed by field name. |

**Returns** `bytes` — The encoded binary data.

### `protobuf::decode`

```kiwi
protobuf::decode(schema: hashmap, data: bytes): hashmap
```

Decodes Protocol Buffer binary data into a Kiwi hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `hashmap` |  | Field descriptor hashmap keyed by field number. |
| `data` | `bytes` |  | The binary data to decode. |

**Returns** `hashmap` — Decoded data keyed by field name.

### `protobuf::encode_file`

```kiwi
protobuf::encode_file(schema: hashmap, data: hashmap, file_path: string)
```

Encodes a protobuf message and writes it to a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `hashmap` |  | Field descriptor hashmap keyed by field number. |
| `data` | `hashmap` |  | Data to encode. |
| `file_path` | `string` |  | Output file path. |

### `protobuf::decode_file`

```kiwi
protobuf::decode_file(schema: hashmap, file_path: string): hashmap
```

Reads a file and decodes it as a protobuf message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `hashmap` |  | Field descriptor hashmap keyed by field number. |
| `file_path` | `string` |  | Path to the binary file. |

**Returns** `hashmap` — Decoded data keyed by field name.

**Throws** `string` — If the file does not exist.

### `protobuf::field`

```kiwi
protobuf::field(name: string, type: string, repeated: boolean = false, schema: hashmap = {}): hashmap
```

Builds a field descriptor hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Field name. |
| `type` | `string` |  | Field type. |
| `repeated` | `boolean` | `false` | Whether the field is repeated (default false). |
| `schema` | `hashmap` | `{}` | Nested schema for message fields (default {}). |

**Returns** `hashmap` — A field descriptor.
