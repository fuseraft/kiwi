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

## Round-Trip Example

```kiwi
original = {id: 1, tags: ["kiwi", "json"], active: true}

encoded = json::stringify(original)
decoded = json::parse(encoded)

println decoded["id"]          # 1
println decoded["tags"].first() # kiwi
println decoded["active"]      # true
```
