---
title: csv
description: Parse CSV text and files into lists or hashmaps, with typed columns.
---

The `csv` package parses comma-separated values following RFC 4180, including quoted fields that contain commas or line breaks.

## Quick parsing

`parse` returns a list of rows, each a list of strings. `to_maps` uses the first row as headers and returns a list of hashmaps.

```kiwi
data = "name,age\nAda,36\n\"Lovelace, A\",37"

println csv::parse(data)
# [["name", "age"], ["Ada", "36"], ["Lovelace, A", "37"]]

for person in csv::to_maps(data) do
  println "${person["name"]} is ${person["age"]}"
end
```

`parse_file` and `file_to_maps` do the same for a file. All four take an optional delimiter:

```kiwi
println csv::parse("a;b\n1;2", ";")   # [["a", "b"], ["1", "2"]]
```

```kiwi nocheck
rows = csv::file_to_maps("people.csv")
tsv = csv::parse_file("report.tsv", "\t")
```

## Pipelines with typed columns

`csv::pipeline()` builds a parse step by step. Every row comes back as a hashmap, and a schema converts columns from strings to the types you need.

```kiwi
data = "name,score,passed\nAlice,95,true\nBob,,false"

rows = csv::pipeline()
  .with_schema(["string", "integer", "boolean"])
  .from_string(data)
  .parse()

for r in rows do
  println "${r["name"]}: ${r["score"] ?? "no score"}"
end
# Alice: 95
# Bob: no score
```

| Method | Effect |
| :-- | :-- |
| `.from_file(path)`, `.from_string(text)` | Add a source. |
| `.with_schema(types)` | Convert each column to a type, by position. |
| `.with_headers(names)` | Use these column names; the source has no header row. |
| `.with_delimiter(text)` | Use a delimiter other than `,`. |
| `.as_dataset(name)` | Name the most recent source. |
| `.parse()` | Run the pipeline. |

### Schema types

| Type | Conversion | Empty field |
| :-- | :-- | :-- |
| `"string"` | Unchanged | `""` |
| `"string?"` | Unchanged | `null` |
| `"integer"` or `"int"` | Parsed as an integer | `null` |
| `"float"`, `"double"` or `"number"` | Parsed as a float | `null` |
| `"boolean"` or `"bool"` | `true` for `"true"`, `"1"` or `"yes"`; otherwise `false` | `false` |
| `"date"` | Parsed as a date | `null` |

Use `"string?"` when an empty text field should count as missing, so that `??` and `!= null` work on it.

### Several sources

With several unnamed sources, `parse()` returns a list with one result per source. Name every source with `.as_dataset()` to get a hashmap instead; mixing named and unnamed sources is an error.

```kiwi
data = csv::pipeline()
  .with_schema(["string", "integer"])
  .from_string("product,qty\nkiwi,3").as_dataset("orders")
  .with_schema(["string", "float"])
  .from_string("product,price\nkiwi,0.5").as_dataset("prices")
  .parse()

println data["orders"]   # [{"product": "kiwi", "qty": 3}]
println data["prices"]   # [{"product": "kiwi", "price": 0.5}]
```

For filtering, joining and grouping parsed rows, see [`table`](/kiwi/stdlib/table/).

<!-- api: csv -->
