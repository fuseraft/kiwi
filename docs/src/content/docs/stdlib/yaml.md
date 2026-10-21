---
title: yaml
description: Parse and produce YAML.
---

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

<!-- api: yaml -->
