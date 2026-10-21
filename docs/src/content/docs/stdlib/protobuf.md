---
title: protobuf
description: Encode and decode Protocol Buffers (proto3) with schemas written as hashmaps.
---

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

<!-- api: protobuf -->
