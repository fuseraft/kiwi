# yaml

> Parse and produce YAML.

The `yaml` package converts between YAML text and Kiwi values: mappings become hashmaps, sequences become lists, and scalars become strings, numbers, booleans or `null`.

```kiwi
config = yaml::parse("
name: kiwi
version: 2
tags:
  - fast
  - fun
server:
  host: localhost
  port: 8080
debug: false
")

println config["server"]["port"]   # 8080
println config["tags"]             # ["fast", "fun"]

println yaml::stringify({name: "kiwi", ports: [80, 443]})
```

`parse_file(path)` reads a file, and `parse_all(text)` reads a stream of several documents separated by `---`, returning a list:

```kiwi
println yaml::parse_all("a: 1\n---\na: 2")   # [{"a": 1}, {"a": 2}]
```

## Supported YAML

- Block mappings and sequences, and inline `{...}` and `[...]` collections.
- Plain, single-quoted and double-quoted scalars, and `|` and `>` block scalars.
- Comments.
- Anchors (`&name`) and aliases (`*name`). The `<<` merge key is not expanded; it is kept as an ordinary key.

Scalars follow the YAML 1.2 core schema: `true`/`false` are booleans, numbers are integers or floats, `null` and `~` are null, and everything else, including `yes`, `no` and dates, is a string. Quote a value to keep it a string.

```kiwi
values = yaml::parse("count: 010\nratio: 1.5e2\nanswer: yes\nid: '42'")
println values   # {"count": 10, "ratio": 150, "answer": "yes", "id": "42"}
```

## Functions

### `yaml::parse`

```kiwi
yaml::parse(input: string): any
```

Parses a YAML string and returns the first document as a Kiwi value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The YAML-formatted string to parse. |

**Returns** `any` — A Kiwi value (hashmap, list, string, integer, float, boolean, or null).

### `yaml::parse_file`

```kiwi
yaml::parse_file(file_path: string): any
```

Parses a YAML file and returns the first document.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `file_path` | `string` |  | Path to the YAML file. |

**Returns** `any` — Parsed value.

### `yaml::parse_all`

```kiwi
yaml::parse_all(input: string): list
```

Parses a YAML string and returns all documents as a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The YAML-formatted string. |

**Returns** `list` — List of parsed documents, one per --- section.

### `yaml::stringify`

```kiwi
yaml::stringify(value: any): string
```

Serializes a Kiwi value to a YAML-formatted string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `value` | `any` |  | The value to serialize. |

**Returns** `string` — YAML-formatted string.

## `YamlParser` struct

A YAML 1.2 parser and serializer written in pure Kiwi. Supports: - Block mappings and sequences (any indentation) - Flow collections: {} and [] - Scalars: null, boolean, integer, float, plain and quoted strings - Single- and double-quoted strings with escape sequences - Block scalars: | (literal) and > (folded) - Inline comments - Anchors (&name) and aliases (*name) - Document markers (--- and ...) - Multi-document streams via parse_all() - Serialization via stringify() Usage: import "yaml_parser/yaml" doc = yaml::parse("name: Kiwi\nversion: 1") println doc["name"]  # => Kiwi

### `YamlParser.new`

```kiwi
YamlParser.new(text)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `text` | `any` |  |  |

### `.parse`

```kiwi
parse()
```

### `.parse_all`

```kiwi
parse_all()
```
