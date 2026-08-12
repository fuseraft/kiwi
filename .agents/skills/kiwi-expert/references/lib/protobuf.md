# `protobuf`

The `protobuf` package provides functions for encoding and decoding data in Google's [Protocol Buffer](https://protobuf.dev/) binary wire format (proto3).

No `.proto` compiler or generated code is needed. Schemas are described directly in Kiwi as hashmaps, and data is encoded or decoded using the standard proto3 wire format — making the output fully interoperable with other protobuf implementations.

```kiwi
import "protobuf"
```

---

## Schema format

A schema is a hashmap keyed by **field number** (integer). Each value is a **field descriptor** hashmap with the following keys:

| Key | Type | Required | Description |
| :--- | :--- | :--- | :--- |
| `name` | `string` | yes | Field name used as the key in data hashmaps. |
| `type` | `string` | yes | Wire type. See [field types](#field-types) below. |
| `repeated` | `boolean` | no | If `true`, the field is a repeated (list) field. Defaults to `false`. |
| `schema` | `hashmap` | no | Nested schema, required when `type` is `"message"`. |

### Field types

| Type string | Proto3 type | Wire encoding |
| :--- | :--- | :--- |
| `"int32"` | `int32` | Varint |
| `"int64"` | `int64` | Varint |
| `"uint32"` | `uint32` | Varint |
| `"uint64"` | `uint64` | Varint |
| `"sint32"` | `sint32` | Varint (zigzag — efficient for negative numbers) |
| `"sint64"` | `sint64` | Varint (zigzag — efficient for negative numbers) |
| `"bool"` | `bool` | Varint |
| `"enum"` | `enum` | Varint |
| `"float"` | `float` | 32-bit fixed |
| `"fixed32"` | `fixed32` | 32-bit fixed |
| `"sfixed32"` | `sfixed32` | 32-bit fixed |
| `"double"` | `double` | 64-bit fixed |
| `"fixed64"` | `fixed64` | 64-bit fixed |
| `"sfixed64"` | `sfixed64` | 64-bit fixed |
| `"string"` | `string` | Length-delimited (UTF-8) |
| `"bytes"` | `bytes` | Length-delimited |
| `"message"` | embedded message | Length-delimited (requires `schema` key) |

> **Tip:** Use `sint32`/`sint64` instead of `int32`/`int64` for fields that often hold negative numbers — the zigzag encoding is much more compact.

---

## Package Functions

### `protobuf::encode(schema, data)`

Encodes a Kiwi hashmap into the Protocol Buffer binary format.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `hashmap` | `schema` | Field descriptor hashmap keyed by field number. |
| `hashmap` | `data` | Data to encode, keyed by field name. Fields not present in `data` are omitted (proto3 default). |

**Returns**

| Type | Description |
| :--- | :--- |
| `bytes` | The encoded binary data. |

**Example**

```kiwi
import "protobuf"

schema = {
  1: { name: "id",   type: "int32"  },
  2: { name: "name", type: "string" },
}

encoded = protobuf::encode(schema, { id: 1, name: "Alice" })
println "encoded ${encoded.size()} bytes"
```

---

### `protobuf::decode(schema, data)`

Decodes Protocol Buffer binary data into a Kiwi hashmap.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `hashmap` | `schema` | Field descriptor hashmap keyed by field number. |
| `bytes` | `data` | The binary data to decode. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | Decoded data keyed by field name. Only fields present in the binary are included (except `repeated` fields, which always decode as a list, empty if absent). |

**Example**

```kiwi
import "protobuf"

schema = {
  1: { name: "id",   type: "int32"  },
  2: { name: "name", type: "string" },
}

encoded = protobuf::encode(schema, { id: 42, name: "Bob" })
decoded = protobuf::decode(schema, encoded)

id   = decoded["id"]
name = decoded["name"]
println "${name} (id=${id})"
# Bob (id=42)
```

---

### `protobuf::encode_file(schema, data, file_path)`

Encodes a hashmap and writes the binary result to a file.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `hashmap` | `schema` | Field descriptor hashmap keyed by field number. |
| `hashmap` | `data` | Data to encode. |
| `string` | `file_path` | Destination file path. |

**Example**

```kiwi
import "protobuf"

schema = { 1: { name: "msg", type: "string" } }
protobuf::encode_file(schema, { msg: "hello" }, "/tmp/out.pb")
```

---

### `protobuf::decode_file(schema, file_path)`

Reads a binary file and decodes it as a protobuf message.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `hashmap` | `schema` | Field descriptor hashmap keyed by field number. |
| `string` | `file_path` | Path to the binary file. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | Decoded data keyed by field name. |

**Throws**

A string error if the file does not exist.

**Example**

```kiwi
import "protobuf"

schema = { 1: { name: "msg", type: "string" } }
decoded = protobuf::decode_file(schema, "/tmp/out.pb")
msg = decoded["msg"]
println msg  # hello
```

---

### `protobuf::field(name, type, repeated, schema)`

Builds a field descriptor hashmap. A convenience alternative to writing the descriptor literal by hand.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | Field name. |
| `string` | `type` | Field type string. |
| `boolean` | `repeated` | Whether the field is repeated (optional, defaults to `false`). |
| `hashmap` | `schema` | Nested schema for `"message"` fields (optional, defaults to `{}`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A field descriptor ready for use in a schema. |

**Example**

```kiwi
import "protobuf"

schema = {
  1: protobuf::field("id",    "int32"),
  2: protobuf::field("name",  "string"),
  3: protobuf::field("tags",  "string", true),   # repeated
}
```

---

## Examples

### Basic scalar roundtrip

```kiwi
import "protobuf"

schema = {
  1: { name: "id",    type: "int32"  },
  2: { name: "score", type: "double" },
  3: { name: "label", type: "string" },
  4: { name: "ok",    type: "bool"   },
}

data = { id: 7, score: 9.81, label: "test", ok: true }

encoded = protobuf::encode(schema, data)
decoded = protobuf::decode(schema, encoded)

id    = decoded["id"]
score = decoded["score"]
label = decoded["label"]
ok    = decoded["ok"]

println "id=${id} score=${score} label=${label} ok=${ok}"
# id=7 score=9.81 label=test ok=true
```

### Repeated fields

```kiwi
import "protobuf"

schema = {
  1: { name: "name",  type: "string" },
  2: { name: "tags",  type: "string", repeated: true },
  3: { name: "scores", type: "int32", repeated: true },
}

data = {
  name:   "Alice",
  tags:   ["dev", "kiwi", "proto"],
  scores: [95, 87, 100],
}

encoded = protobuf::encode(schema, data)
decoded = protobuf::decode(schema, encoded)

tags   = decoded["tags"]
scores = decoded["scores"]
println tags    # ["dev", "kiwi", "proto"]
println scores  # [95, 87, 100]
```

### Nested messages

```kiwi
import "protobuf"

address_schema = {
  1: { name: "street", type: "string" },
  2: { name: "city",   type: "string" },
  3: { name: "zip",    type: "int32"  },
}

person_schema = {
  1: { name: "name",    type: "string" },
  2: { name: "age",     type: "int32"  },
  3: { name: "address", type: "message", schema: address_schema },
}

person = {
  name: "Bob",
  age:  42,
  address: {
    street: "1 Protobuf Lane",
    city:   "Kiwi Town",
    zip:    12345,
  },
}

encoded = protobuf::encode(person_schema, person)
decoded = protobuf::decode(person_schema, encoded)

addr   = decoded["address"]
name   = decoded["name"]
street = addr["street"]
city   = addr["city"]
zip    = addr["zip"]

println "${name} lives at ${street}, ${city} ${zip}"
# Bob lives at 1 Protobuf Lane, Kiwi Town 12345
```

### File I/O

```kiwi
import "protobuf"

schema = {
  1: { name: "id",    type: "int32"  },
  2: { name: "value", type: "double" },
}

# Write
protobuf::encode_file(schema, { id: 1, value: 3.14 }, "/tmp/record.pb")

# Read back
rec   = protobuf::decode_file(schema, "/tmp/record.pb")
id    = rec["id"]
value = rec["value"]
println "id=${id} value=${value}"
# id=1 value=3.14
```

### Using `protobuf::field()`

```kiwi
import "protobuf"

coord_schema = {
  1: { name: "x", type: "double" },
  2: { name: "y", type: "double" },
}

track_schema = {
  1: protobuf::field("name",   "string"),
  2: protobuf::field("points", "message", true, coord_schema),
}

track = {
  name: "route-1",
  points: [
    { x: 1.0, y: 2.0 },
    { x: 3.5, y: 4.5 },
    { x: 6.0, y: 0.5 },
  ],
}

encoded = protobuf::encode(track_schema, track)
decoded = protobuf::decode(track_schema, encoded)

pts  = decoded["points"]
name = decoded["name"]
println "${name}: ${pts.size()} points"
# route-1: 3 points
```
