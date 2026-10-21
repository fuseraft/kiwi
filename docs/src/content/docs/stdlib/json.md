---
title: json
description: Parse and produce JSON, and load JSON records with typed fields.
---

The `json` package converts between JSON text and Kiwi values. JSON objects become hashmaps, arrays become lists, and `null`, booleans and numbers map to their Kiwi equivalents.

```kiwi
text = json::stringify({id: 1, tags: ["kiwi", "json"], active: true})
println text                  # {"id":1,"tags":["kiwi","json"],"active":true}

data = json::parse(text)
println data["tags"][0]       # kiwi

println json::stringify({name: "kiwi", size: 3}, true)
```

Pass `true` as the second argument to `stringify` for indented output. Reading and writing files is a matter of combining `json` with [`fio`](/kiwi/stdlib/fio/):

```kiwi nocheck
config = json::parse(fio::read("config.json"))
fio::write("config.json", json::stringify(config, true))
```

## Pipelines

`json::pipeline()` loads JSON records (an array of objects, or a single object) with field selection and type conversion. It mirrors the [`csv`](/kiwi/stdlib/csv/#pipelines-with-typed-columns) pipeline.

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
| `.with_schema(types)` | Convert fields by name in the sources added after it; the types are the same as for [CSV](/kiwi/stdlib/csv/#schema-types). |
| `.as_dataset(name)` | Name the most recent source. |
| `.parse()` | Run the pipeline. |

A single top-level object is returned as a one-element list. With named datasets, `parse()` returns a hashmap of lists.

```kiwi nocheck
data = json::pipeline()
  .from_file("users.json").as_dataset("users")
  .from_file("roles.json").as_dataset("roles")
  .parse()
```

<!-- api: json -->
