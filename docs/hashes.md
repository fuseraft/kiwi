# Hashes

Hashes in Kiwi are similar to maps and dictionaries in other languages.

A hash just contains a series of key-value pairs.

### Defining a Hash

The keys in a hash must be unique strings.

```ruby
@myHash = {"key1": true, "key2": 1, "key2": [1, 2, 3, 4]}
```

### Accessing Hash Elements

Bracket notation can be used to access elements by key.

```ruby
println @myHash["key2"]  # Outputs: [1, 2, 3, 4]
```

### Adding Elements to a Hash

```ruby
@myHash = {}
@myHash["key1"] = [1..3]

# @myHash now contains {"key1": [1, 2, 3]}
```

### Removing Elements from a Hash

You can use the `delete` keyword to remove an element of a hash by key.

```ruby
@hash = {"key1": 1, "key2": true, "key3": [1, 2, 3]}
delete @hash["key2"]
println @hash # prints: {"key1": 1, "key3": [1, 2, 3]}
```

### Iterating a Hash

Use the `for` keyword and the `.keys()` Hash-builtin to iterate a hash.

```ruby
# Iterate the values in the list.
for @key in @myHash.keys() do
  println @myHash[@key]
end

# Iterate the values in the list, with an index.
for @key, @index in @myHash.keys() do
  println "Key ${@index}: ${@key}"
end
```

### Optional Commas

Commas are optional in Kiwi.

```ruby
@hash = {
  "A": {
    "B": {
      "C": {}
      "D": {}
    }
    "E": {
      "F": {}
      "G": [1 2 3]
    }
  }
}

println "
hash = ${@hash}
"

# prints: 
#
# hash = {"A": {"B": {"C": {}, "D": {}}, "E": {"F": {}, "G": [1, 2, 3]}}}
#
```