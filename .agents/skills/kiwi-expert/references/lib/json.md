# json

> Parse and produce JSON, and load JSON records with typed fields.

The `json` package converts between JSON text and Kiwi values. JSON objects become hashmaps, arrays become lists, and `null`, booleans and numbers map to their Kiwi equivalents.

```kiwi
text = json::stringify({id: 1, tags: ["kiwi", "json"], active: true})
println text                  # {"id":1,"tags":["kiwi","json"],"active":true}

data = json::parse(text)
println data["tags"][0]       # kiwi

println json::stringify({name: "kiwi", size: 3}, true)
```

Pass `true` as the second argument to `stringify` for indented output. Reading and writing files is a matter of combining `json` with [`fio`](fio.md):

```kiwi
config = json::parse(fio::read("config.json"))
fio::write("config.json", json::stringify(config, true))
```

## Pipelines

`json::pipeline()` loads JSON records (an array of objects, or a single object) with field selection and type conversion. It mirrors the [`csv`](csv.md#pipelines-with-typed-columns) pipeline.

```kiwi
rows = json::pipeline()
  .select(["id", "score", "active"])
  .with_schema({"id": "integer", "score": "float", "active": "boolean"})
  .from_string('[{"id": "1", "score": "9.5", "active": "true", "note": "x"}]')
  .parse()

println rows   # [{"id": 1, "score": 9.5, "active": true}]
```

| Method | Effect |
| :-- | :-- |
| `.from_file(path)`, `.from_string(text)` | Add a source. |
| `.select(fields)` | Keep only these fields in the sources added after it. |
| `.with_schema(types)` | Convert fields by name in the sources added after it; the types are the same as for [CSV](csv.md#schema-types). |
| `.as_dataset(name)` | Name the most recent source. |
| `.parse()` | Run the pipeline. |

A single top-level object is returned as a one-element list. With named datasets, `parse()` returns a hashmap of lists.

```kiwi
data = json::pipeline()
  .from_file("users.json").as_dataset("users")
  .from_file("roles.json").as_dataset("roles")
  .parse()
```

## Functions

### `json::parse`

```kiwi
json::parse(input: string): any
```

Parses a JSON string into a Kiwi value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The JSON-formatted string to parse. |

**Returns** `any` — A Kiwi value (hashmap, list, string, integer, float, boolean, or null).

### `json::stringify`

```kiwi
json::stringify(value: any, pretty: boolean = false): string
```

Serializes a Kiwi value to a JSON string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `value` | `any` |  | The value to serialize. |
| `pretty` | `boolean` | `false` | Whether to pretty-print the output (default `false`). |

**Returns** `string` — A JSON-formatted string.

### `json::pipeline`

```kiwi
json::pipeline(): JsonPipeline
```

Creates a new JsonPipeline builder for fluent JSON loading.

**Returns** `JsonPipeline` — A new pipeline builder instance.

## `JsonPipeline` struct

A fluent builder for loading and selecting fields from one or more JSON sources.

```kiwi
# Single file
users = json::pipeline().from_file("users.json").parse()

# Select specific fields
rows = json::pipeline()
  .select(["id", "name"])
  .from_file("users.json")
  .parse()

# Multiple named datasets
data = json::pipeline()
  .from_file("users.json").as_dataset("users")
  .from_file("roles.json").as_dataset("roles")
  .parse()
```

### `JsonPipeline.new`

```kiwi
JsonPipeline.new()
```

Initialises an empty pipeline builder.

### `.select`

```kiwi
select(keys: list): JsonPipeline
```

Specifies which keys to retain from each object in the next source.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `keys` | `list` |  | List of key name strings to keep. |

**Returns** `JsonPipeline` — self, for chaining.

### `.with_schema`

```kiwi
with_schema(schema: hashmap): JsonPipeline
```

Sets a type-coercion schema for the next source.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `hashmap` |  | Map of field name to type string. |

**Returns** `JsonPipeline` — self, for chaining.

### `.from_file`

```kiwi
from_file(path: string): JsonPipeline
```

Registers a JSON file as a source using the current pending settings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the JSON file. |

**Returns** `JsonPipeline` — self, for chaining.

**Throws** `string` — If the file does not exist at parse time.

### `.from_string`

```kiwi
from_string(input: string): JsonPipeline
```

Registers a JSON string as a source using the current pending settings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | A JSON-formatted string. |

**Returns** `JsonPipeline` — self, for chaining.

### `.as_dataset`

```kiwi
as_dataset(name: string): JsonPipeline
```

Names the most recently registered source as a dataset.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The dataset key name. |

**Returns** `JsonPipeline` — self, for chaining.

**Throws** `string` — If called before any source is registered.

### `.parse`

```kiwi
parse()
```

Executes the pipeline and returns parsed results.

**Returns** `list|hashmap` — Parsed JSON data.

**Throws** `string` — On missing file or mixed dataset/non-dataset sources.
