# Hashmaps in Kiwi

Hashmaps (also called dictionaries) are unordered, mutable collections of key-value pairs.  
Keys must be unique and are typically strings (or other hashable types). Values can be any Kiwi type.

Hashmaps are the natural way to represent structured data and are directly serializable to/from JSON.

## Quick Reference – Common Operations

| Operation                  | Syntax / Method                          | Returns          | Mutates? |
|----------------------------|------------------------------------------|------------------|----------|
| Create                     | `{}`, `{"a": 1, "b": true}`              | `hashmap`        | —        |
| Get value                  | `map["key"]`, `map.key`                  | value or `null`  | No       |
| Set value                  | `map["key"] = val`, `map.key = val`      | —                | Yes      |
| Check key exists           | `map.contains("key")`, `"key" in map`    | `boolean`        | No       |
| Get with default           | `map.get("key", default)`                | value            | No       |
| Remove key                 | `map.remove("key")` or `map.delete("key")`| removed value    | Yes      |
| Size                       | `map.size()` or `map.length()`           | integer          | No       |
| Iterate pairs              | `for k, v in map do …`                   | —                | —        |
| Iterate keys               | `for k in map.keys() do …`               | —                | —        |
| Iterate values             | `for v in map.values() do …`             | —                | —        |

For the complete list of built-in methods, see [Hashmap Builtins](builtins.md#hashmap-builtins).

## Creating Hashmaps

### Literal syntax

```kiwi
empty   = {}
person  = {
  name:    "Alice",
  age:     28,
  active:  true,
  hobbies: ["reading", "hiking", "coding"]
}
```

**Note:** Duplicate keys in a literal are **not allowed** — the last one wins (but avoid this).

```kiwi
# Bad (last value overwrites)
bad = {"a": 1, "a": 2}   # -> {"a": 2}
```

## Accessing Values

### Bracket notation (always works)

```kiwi
println person["name"]     # "Alice"
println person["age"]      # 28
```

### Dot notation (only for valid identifier keys)

```kiwi
println person.name        # "Alice"
println person.age         # 28
```

**Dot notation limitations**
- Cannot be used for keys with spaces, hyphens, numbers at start, or special characters.
- Cannot be used when the key is stored in a variable.

```kiwi
key = "favorite-color"
println person[key]        # works
# println person.favorite-color   # syntax error
```

**Missing keys**  
Both notations return `null` if the key doesn't exist (safe access).

```kiwi
println person["height"]   # null
println person.height      # null
```

**Safe access with default**

```kiwi
level = person.get("level", "beginner")   # "beginner" if missing
```

## Adding / Updating Entries

```kiwi
settings = {}

# Bracket (most flexible)
settings["theme"] = "dark"
settings["font-size"] = 14

# Dot (clean for simple keys)
settings.autoSave = true
settings.maxRetries = 3

println settings
# -> {"theme": "dark", "font-size": 14, "autoSave": true, "maxRetries": 3}
```

## Removing Entries

```kiwi
config = {
  apiKey:    "abc123",
  debug:     true,
  timeout:   30000,
  logLevel:  "info"
}

config.remove("debug")
# config is now {"apiKey": "abc123", "timeout": 30000, "logLevel": "info"}
```

## Checking for Keys

```kiwi
user = { name: "Bob", email: "bob@example.com" }

println user.has_key("name")      # true
println "email" in user            # true (alternative syntax)
println user.has_key("phone")     # false
```

## Iterating Hashmaps

### Key-value pairs (most common)

```kiwi
scores = { alice: 95, bob: 82, carol: 100 }

for name, score in scores do
  println "${name}: ${score}"
end
```

### Only keys

```kiwi
for name in scores.keys() do
  println name
end
```

### Only values

```kiwi
for score in scores.values() do
  println score
end
```

### With index (if order matters — insertion order is preserved)

```kiwi
for name, score, idx in scores do
  println "${idx + 1}. ${name} -> ${score}"
end
```

## Full Example – Simple Config Manager

```kiwi
config = {
  server:   "localhost",
  port:     8080,
  debug:    false,
  retries:  3
}

fn load_config(file)
  # Simulate loading from file
  return { debug: true, logLevel: "verbose" }
end

# Merge defaults + overrides
overrides = load_config("config.json")
for k, v in overrides do
  config[k] = v
end

println "Final config:"
for k, v in config do
  println "  ${k}: ${v}"
end
```

## Best Practices & Tips

- Use **dot notation** when keys are simple identifiers -> cleaner code
- Use **bracket notation** when keys are dynamic/variables or contain special chars
- Prefer `.get(key, default)` over direct access when absence is expected
- Avoid very large keys or deeply nested structures without good reason
- Hashmaps preserve **insertion order** (since Kiwi 1.x) — useful for config files, JSON round-tripping
- To merge two maps: loop with `for k, v in source do target[k] = v`

See also:
- [Hashmap Builtins](builtins.md#hashmap-builtins)
- [Loops – `for` on hashmaps](loops.md#hashmaps)
- [Control Flow](control_structures.md#3-when--guard-clauses--conditional-modifiers) (guards with `when`)