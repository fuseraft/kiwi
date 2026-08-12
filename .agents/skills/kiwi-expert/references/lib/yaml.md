# `yaml`

The `yaml` package provides a pure-Kiwi YAML 1.2 parser and serializer.

**Supported features:**
- Block mappings and sequences (any indentation)
- Flow collections: `{}` and `[]`
- Scalars: `null`, boolean, integer, float, plain and quoted strings
- Single- and double-quoted strings with escape sequences
- Block scalars: `|` (literal) and `>` (folded)
- Inline comments
- Anchors (`&name`) and aliases (`*name`)
- Document markers (`---` and `...`)
- Multi-document streams via `parse_all()`
- Serialization via `stringify()`

---

## Package Functions

### `parse(input)`

Parses a YAML string and returns the first document as a Kiwi value.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | The YAML-formatted string to parse. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | A Kiwi value: `hashmap`, `list`, `string`, `integer`, `float`, `boolean`, or `null`. |

**Example**

```kiwi
doc = yaml::parse("name: Kiwi\nversion: 2\nactive: true")
println doc["name"]    # Kiwi
println doc["version"] # 2
println doc["active"]  # true
```

---

### `parse_file(file_path)`

Parses a YAML file and returns the first document.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `file_path` | Path to the YAML file. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | The parsed document. |

**Throws**

`string` — if the file does not exist.

**Example**

```kiwi
doc = yaml::parse_file("/etc/myapp/config.yaml")
println doc["database"]["host"]
```

---

### `parse_all(input)`

Parses a YAML string containing multiple `---`-separated documents and returns all of them as a list.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | A YAML string with one or more documents. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | One parsed document per `---` section. |

**Example**

```kiwi
src = "---\nname: first\n---\nname: second"
docs = yaml::parse_all(src)
println docs.size()       # 2
println docs[0]["name"]   # first
println docs[1]["name"]   # second
```

---

### `stringify(value)`

Serializes a Kiwi value to a YAML-formatted string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `value` | The value to serialize (`hashmap`, `list`, scalar, or `null`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | YAML-formatted string. |

**Example**

```kiwi
data = {name: "Alice", scores: [95, 87, 100], active: true}
println yaml::stringify(data)
# name: Alice
# scores:
#   - 95
#   - 87
#   - 100
# active: true
```

---

## Examples

### Nested mapping

```kiwi
src = "
server:
  host: localhost
  port: 8080
  tls: false
"
cfg = yaml::parse(src)
host = cfg["server"]["host"]
port = cfg["server"]["port"]
println "${host}:${port}"  # localhost:8080
```

### Sequence of mappings

```kiwi
src = "
- name: Alice
  age: 30
- name: Bob
  age: 25
"
people = yaml::parse(src)
for p in people do
  n = p["name"]
  a = p["age"]
  println "${n} is ${a}"
end
# Alice is 30
# Bob is 25
```

### Anchors and aliases

```kiwi
src = "
defaults: &defaults
  timeout: 30
  retries: 3

production:
  <<: *defaults
  host: prod.example.com
"
doc = yaml::parse(src)
println doc["defaults"]["timeout"]  # 30
```

### Round-trip (parse → stringify → parse)

```kiwi
original = {project: "Kiwi", version: 2, tags: ["scripting", "aot"]}
serialized = yaml::stringify(original)
restored = yaml::parse(serialized)
tags = restored["tags"]
println tags[0]  # scripting
```

### Scalar type coercion

YAML scalars are automatically coerced to Kiwi types:

| YAML literal | Kiwi type | Value |
| :--- | :--- | :--- |
| `42` | `integer` | `42` |
| `3.14` | `float` | `3.14` |
| `true` / `True` / `TRUE` | `boolean` | `true` |
| `false` / `False` / `FALSE` | `boolean` | `false` |
| `null` / `~` / `Null` | `null` | `null` |
| `"hello"` | `string` | `"hello"` |
