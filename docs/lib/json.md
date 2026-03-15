# `json`

The `json` package provides functions for parsing and serializing JSON data. It is automatically loaded by the Kiwi runtime — no explicit import is needed.

```kiwi
# json is pre-loaded; no import required.
# To import explicitly:
import "json"
```

---

## Functions

### `json::parse(input)`

Parses a JSON string into a Kiwi value.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | A JSON-formatted string. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | A Kiwi value: `hashmap`, `list`, `string`, `integer`, `float`, `boolean`, or `null`. |

**Example**

```kiwi
data = json::parse('{"name": "Alice", "age": 30}')
println data["name"]  # Alice
println data["age"]   # 30

items = json::parse('[1, 2, 3]')
println items.first()  # 1
```

---

### `json::stringify(value, pretty?)`

Serializes a Kiwi value to a JSON string.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `any` | `value` | The value to serialize. | |
| `boolean` | `pretty` | Whether to pretty-print the output. | `false` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A JSON-formatted string. |

**Example**

```kiwi
data = {name: "Alice", scores: [95, 87, 92]}
println json::stringify(data)
# {"name":"Alice","scores":[95,87,92]}

println json::stringify(data, true)
# {
#   "name": "Alice",
#   "scores": [
#     95,
#     87,
#     92
#   ]
# }
```

---

### `json::pipeline()`

Creates a `JsonPipeline` builder for fluent, declarative JSON loading. Chain configuration methods to describe your sources, then call `.parse()` to execute.

**Returns**

| Type | Description |
| :--- | :--- |
| `JsonPipeline` | A new pipeline builder instance. |

See [JsonPipeline](#jsonpipeline) below for the full builder API and examples.

---

## `JsonPipeline`

A fluent builder returned by `json::pipeline()`. Configure one or more JSON sources, then call `.parse()` to get structured data.

All builder methods return `self` so calls can be chained.

### `.select(keys)`

Retains only the specified fields from each object in the **next** registered source.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `keys` | Field names to keep. |

### `.from_file(path)`

Registers a JSON file as a source using any pending `.select()` setting.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | Path to the JSON file. |

**Throws** — if the file does not exist at parse time.

### `.from_string(input)`

Registers a JSON string as a source.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | A JSON-formatted string. |

### `.as_dataset(name)`

Names the most recently registered source. The name becomes the key in the hashmap returned by `.parse()`. Must be called after `.from_file()` or `.from_string()`.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | Dataset key name. |

**Throws** — if called before any source is registered.

### `.parse()`

Executes the pipeline and returns the parsed results. JSON arrays become lists of hashmaps; a JSON object at the top level is wrapped in a one-element list automatically.

| Condition | Return type |
| :--- | :--- |
| Single unnamed source | `list` of hashmaps |
| Multiple unnamed sources | `list` of lists of hashmaps |
| All sources named as datasets | `hashmap` keyed by dataset name |
| Mixed named/unnamed | throws a pipeline error |

---

## `JsonPipeline` Examples

### Single file

```kiwi
users = json::pipeline().from_file("users.json").parse()

for u in users do
  println u["name"]
end
```

### Field selection

```kiwi
rows = json::pipeline()
  .select(["id", "name"])
  .from_file("users.json")
  .parse()

# Each row has only "id" and "name"
```

### Multiple named datasets

```kiwi
data = json::pipeline()
  .from_file("users.json").as_dataset("users")
  .from_file("roles.json").as_dataset("roles")
  .parse()

users = data["users"]
roles = data["roles"]
```

### From string

```kiwi
rows = json::pipeline()
  .from_string('[{"x": 1}, {"x": 2}]')
  .parse()

println rows[0]["x"]  # 1
```

### Single JSON object (auto-wrapped)

A top-level JSON object is automatically wrapped in a one-element list so the return type is always a `list` when no datasets are named.

```kiwi
rows = json::pipeline().from_file("config.json").parse()
cfg = rows[0]
println cfg["version"]
```

---

## Round-Trip Example

```kiwi
original = {id: 1, tags: ["kiwi", "json"], active: true}

encoded = json::stringify(original)
decoded = json::parse(encoded)

println decoded["id"]          # 1
println decoded["tags"].first() # kiwi
println decoded["active"]      # true
```
