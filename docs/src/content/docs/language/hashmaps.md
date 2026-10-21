---
title: Hashmaps
description: "Key-value maps: literals, access, updates, iteration and nesting."
---

A hashmap maps keys to values. Keys are unique and keep their insertion order, which makes hashmaps a good fit for records, configuration, and anything you round-trip through JSON.

```kiwi
person = {
  name: "Alice",
  age: 28,
  hobbies: ["reading", "hiking"]
}

println person   # {"name": "Alice", "age": 28, "hobbies": ["reading", "hiking"]}
```

A bare identifier before the colon is a string key, so `{name: "Alice"}` and `{"name": "Alice"}` are the same. Quote keys that aren't valid identifiers, or that are [reserved words](/kiwi/language/syntax/#reserved-words) such as `list` or `string`. Integers and booleans work as keys too.

```kiwi
headers = {"Content-Type": "text/plain", "list": [1]}
codes = {200: "OK", 404: "Not Found"}
println codes[404]   # Not Found
```

If a literal repeats a key, the last value wins.

## Reading values

Use brackets with any key, or a dot with a key that is a valid identifier. A missing key reads as `null`.

```kiwi
person = {name: "Alice", age: 28}

println person["name"]    # Alice
println person.age        # 28
println person.email      # null

key = "name"
println person[key]       # Alice
```

`.get(key, default)` returns a default for missing keys, and `.has_key(key)` or `in` tests for a key:

```kiwi
settings = {theme: "dark"}

println settings.get("font_size", 14)   # 14
println settings.has_key("theme")       # true
println "theme" in settings             # true
```

`.has_key` is the reliable test when a key may be present with a `null` value; `settings.key ?? fallback` can't tell the two apart.

## Adding, updating and removing

```kiwi
settings = {}

settings["theme"] = "dark"
settings.font_size = 14
settings.set("autosave", true)
println settings          # {"theme": "dark", "font_size": 14, "autosave": true}

settings.remove("autosave")
println settings.keys()   # ["theme", "font_size"]

settings.merge({theme: "light", tabs: 2})
println settings          # {"theme": "light", "font_size": 14, "tabs": 2}
```

`.merge(other)` copies every entry of `other` into the hashmap, overwriting keys that exist. `.clear()` removes everything.

## Iterating

A `for` loop with two variables walks key-value pairs in insertion order. With one variable, it walks the keys.

```kiwi
scores = {alice: 95, bob: 82, carol: 100}

for name, score in scores do
  println "${name}: ${score}"
end

for name in scores do
  print "${name} "
end
println ""

println scores.keys()     # ["alice", "bob", "carol"]
println scores.values()   # [95, 82, 100]
println scores.size()     # 3
```

## Nesting

Hashmaps and lists nest freely. Chained access and assignment reach into nested values directly:

```kiwi
config = {
  server: {host: "localhost", port: 8080},
  features: ["auth", "logging"]
}

println config.server.port         # 8080
println config["features"][0]      # auth

config.server.port = 9090
config["features"].push("metrics")
println config
```

## Hashmaps are shared

Like lists, hashmaps are passed by reference. Assigning one to another variable or reading it out of another collection gives you the same hashmap, not a copy. Use `.clone()` for an independent deep copy.

```kiwi
defaults = {retries: 3}
options = defaults
options.retries = 5
println defaults.retries   # 5

safe = defaults.clone()
safe.retries = 1
println defaults.retries   # 5
```

## Converting

`.pretty()` returns an indented, JSON-style string, and [`serialize`](/kiwi/reference/global-functions/#serialize) returns a compact one. For JSON input and output, use the [`json`](/kiwi/stdlib/json/) package.

```kiwi
data = {name: "kiwi", tags: ["fruit"]}
println data.pretty()
```

The [Built-in Methods](/kiwi/reference/builtin-methods/#hashmaps) reference lists every hashmap method.
