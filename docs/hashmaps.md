# Hashmaps

Hashmaps in Kiwi are similar to hashmaps and dictionaries in other languages.

A hashmap contains a series of key-value pairs and can easily be serialized into JSON and vice-versa.

# Table of Contents
- [Builtins](#hashmap-builtins)
- [Defining a `Hashmap`](#defining-a-hashmap)
- [Accessing Elements](#accessing-hashmap-elements)
- [Adding Elements](#adding-elements-to-a-hashmap)
- [Removing Elements](#removing-elements-from-a-hashmap)
- [Iterating a `Hashmap`](#iterating-a-hashmap)

### Builtins

For documentation on `Hashmap` builtins, take a look at the [`Hashmap` builtins](builtins.md#hashmap-builtins).

### Defining a Hashmap

The keys in a hashmap must be unique.

```kiwi
myHashmap = {"key1": true, "key2": 1, "key2": [1, 2, 3, 4]}
```

### Accessing Hashmap Elements

Bracket notation can be used to access elements by key.

```kiwi
println(myHashmap["key2"])  # Outputs: [1, 2, 3, 4]
```

Dot notation can be used to access elements by key.

```kiwi
println(myHashmap.key2)  # Outputs: [1, 2, 3, 4]
```

### Adding Elements to a Hashmap

```kiwi
myHashmap = {}
myHashmap["key1"] = [1, 2, 3]
myHashmap.key2 = { "nested": "hashmap" }

# myHashmap now contains {"key1": [1, 2, 3]}
```

### Removing Elements from a Hashmap

You can use the `delete` keyword to remove an element of a hashmap by key.

```kiwi
hashmap = {"key1": 1, "key2": true, "key3": [1, 2, 3]}
delete hashmap["key2"]
println(hashmap) # prints: {"key1": 1, "key3": [1, 2, 3]}
```

### Iterating a Hashmap

Use the `for` keyword and the `.keys()` Hashmap-builtin to iterate a hashmap.

```kiwi
# Iterate the values in the list.
for key in myHashmap.keys() do
  println(myHashmap[key])
end

# Iterate the values in the list, with an index.
for key, index in myHashmap.keys() do
  println("Key ${index}: ${key}")
end
```