# Builtins

In Kiwi, builtins are accessed using dot-notation and can be used to query or manipulate values and types.

# Table of Contents
- [**`global`**](#global)
- [**`String` Builtins**](#string-builtins)
  - [`begins_with(str)`](#begins_withstr)
  - [`chars()`](#chars)
  - [`contains(str)`](#containsstr)
  - [`lowercase()`](#lowercase)
  - [`ends_with(str)`](#ends_withstr)
  - [`index(str)`](#indexstr)
  - [`lastindex(str)`](#lastindexstr)
  - [`ltrim()`](#ltrim)
  - [`rtrim()`](#rtrim)
  - [`substring(pos, length)`](#substringpos-length)
  - [`trim()`](#trim)
  - [`uppercase()`](#uppercase)
  - [**Regular Expression Builtins**](#regex-builtins)
    - [`find(regex)`](#findregex)
    - [`match(regex)`](#matchregex)
    - [`matches(regex)`](#matchesregex)
    - [`matches_all(regex)`](#matches_allregex)
    - [`replace(search, replacement)`](#replacesearch-replacement)
    - [`scan(regex)`](#scanregex)
    - [`split(delim)`](#splitdelim-limit---1)
- [**`Hash` Builtins**](#hash-builtins)
  - [`keys()`](#keys)
  - [`values()`](#values)
  - [`has_key(key)`](#has_keykey)
  - [`get(key)`](#getkey)
  - [`set(key, value)`](#setkey-value)
  - [`merge(hash)`](#mergehash)
- [**`List` Builtins**](#list-builtins)
  - [`clear()`](#clear)
  - [`concat(list)`](#concatlist)
  - [`count(value)`](#countvalue)
  - [`dequeue()`](#dequeue)
  - [`each(lambda)`](#eachlambda)
  - [`enqueue(value)`](#enqueuevalue)
  - [`first(default_value)`](#firstdefault_value)
  - [`flatten()`](#flatten)
  - [`index(value)`](#indexvalue)
  - [`insert(value, index)`](#insertvalue-index)
  - [`join(str)`](#joinstr)
  - [`last(default_value)`](#lastdefault_value)
  - [`lastindex(value)`](#lastindexvalue)
  - [`map(lambda)`](#maplambda)
  - [`max()`](#max)
  - [`min()`](#min)
  - [`pop()`](#pop)
  - [`push(value)`](#pushvalue)
  - [`reduce(accumulator, lambda)`](#reduceaccumulator-lambda)
  - [`remove(value)`](#removevalue)
  - [`remove_at(index)`](#remove_atindex)
  - [`reverse()`](#reverse)
  - [`rotate(n)`](#rotaten)
  - [`select(lambda)`](#selectlambda)
  - [`shift()`](#shift)
  - [`size()`](#size)
  - [`slice(start, end)`](#slicestart-end)
  - [`sort()`](#sort)
  - [`sum()`](#sum)
  - [`to_bytes()`](#to_bytes)
  - [`to_hex()`](#to_hex)
  - [`unique()`](#unique)
  - [`unshift(value)`](#unshiftvalue)
  - [`zip(list)`](#ziplist)
- [**Conversion and Type Checking**](#conversion-and-type-checking)
  - [`deserialize(str)`](#deserializestr)
  - [`empty()`](#empty)
  - [`clone()`](#clone)
  - [`is_a(type_name)`](#is_atype_name)
  - [`pretty()`](#pretty)
  - [`serialize(value)`](#serializevalue)
  - [`to_double()`](#to_double)
  - [`to_hash()`](#to_hash)
  - [`to_int()`](#to_int)
  - [`to_string()`](#to_string)
  - [`truthy()`](#truthy)
  - [`type()`](#type)

## `global`

The `global` variable is a hash that can be used to store global data. This is useful for sharing data between scripts.

## String Builtins

### `begins_with(str)`

Returns true if the string begins with a given string.

```ruby
println("foobar".begins_with("foo"))   # prints: true
println("foobar".begins_with("food"))  # prints: false
```

### `chars()`

Converts a string into a list. Each character in the string becomes a new string in the list.

```ruby
string = "Hello"
chars = string.chars() 
# chars = ["H", "e", "l", "l", "o"]

println("kiwi".chars()) # prints: ["a", "s", "t", "r", "a", "l"]
```

### `contains(str)`

Returns true if the string contains a given string.

```ruby
println("foobar".contains("bar"))   # prints: true
println("foobar".contains("bark"))  # prints: false
```

### `lowercase()`

Returns the lowercase value of a string.

```ruby
println("FOOBAR".lowercase())   # prints: foobar
```

### `ends_with(str)`

Returns true if the string ends with a given string.

```ruby
println("foobar".ends_with("bar"))   # prints: true
println("foobar".ends_with("bark"))  # prints: false
```

### `index(str)`

Returns the index of a string. Returns -1 if not found.

```ruby
println("foobar".index("bar"))     # prints: 3
println("foobar".index("kiwi"))  # prints: -1
```

### `lastindex(str)`

Returns the last index of a string. Returns -1 if not found.

```ruby
println("foobarbar".lastindex("bar"))  # prints: 6
println("foobar".lastindex("kiwi"))  # prints: -1
```

### `ltrim()`

Trims whitespace from the left-hand side of a string.

```ruby
println("   Hello World!".ltrim()) # prints: Hello World!
```

### `rtrim()`

Trims whitespace from the right-hand side of a string.

```ruby
println("Hello World!    ".rtrim() + " Testing!") # prints: Hello World! Testing!
```

### `trim()`

Trims whitespace from both sides of a string.

```ruby
println("     Hello World!    ".trim() + " Testing!") # prints: Hello World! Testing!
```

### `uppercase()`

Returns the uppercase value of a string.

```ruby
println("foobar".uppercase())  # prints: FOOBAR
```

### `substring(pos, length)`

Extract a substring from a string.

```ruby
println("hello".substring(1))    # prints: ello
println("hello".substring(1, 2)) # prints: el
```

## Regex Builtins

### `find(regex)`

Searches for the first occurrence of a pattern described by a regex and returns the substring.

```ruby
println("my email: example@test.com".find('\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z0-9]{2,}\b'))
# prints: example@test.com
```

### `match(regex)`

Returns the capture groups for the first match of the regex in the string. 

This function extracts parts of the string that match the given regular expression, specifically the groups defined within the pattern.

```ruby
println("June 24, 2021".match('(\w+) (\d+), (\d+)')) # prints: ["June", "24", "2021"]
```

### `matches(regex)`

Tests whether the entire string conforms to a regular expression pattern.

```ruby
println("hello123".matches('^([a-z]+\d{3})$'))   # prints: true
println("hello123!".matches('^([a-z]+\d{3})$'))  # prints: false
```

### `matches_all(regex)`

Checks if all parts of the string conform to the regex pattern.

```ruby
println("123-456-7890".matches_all('\d{3}-\d{3}-\d{4}'))  # prints: true
println("123-456-789x".matches_all('\d{3}-\d{3}-\d{4}'))  # prints: false
```

### `replace(search, replacement)`

Search for a string and replace with a given string.

```ruby
println("foobar".replace("foo", "food"))      # prints: foodbar
println("foo123bar".replace('(\d+)', '[$1]')) # prints: foo[123]bar
println("foo123bar456".replace('\d+', "-"))   # prints: foo-bar-
```

### `scan(regex)`

Finds every occurrence of the regex in the string and returns a list of matches.

```ruby
println("s7s s8s s9s".scan('\d'))  # prints: ["7", "8", "9"]
```

### `split(delim, limit = -1)`

Splits a string into a list by delimiter.

```ruby
println("Hello World!".split(" ")) # prints: ["Hello", "World!"]
println("one,two,three,four".split(",", 2))  # prints: ["one", "two,three,four"]
```

## Hash Builtins

### `keys()`

Returns the list of keys from a hash.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println(hash.keys()) # prints: ["key1", "key2", "key3"]
```

### `has_key(key)`

Returns true if a hash contains a given key.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println(hash.has_key("key2")) # prints: true
```

### `get(key)`

Returns the value assigned to a given key.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println(hash.get("key3")) # prints: ["a", "b", "c"]
```

### `set(key, value)`

Sets the value assigned to a given key.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

hash.set("key3", 31337)
println(hash.get("key3")) # prints: 31337
```

### `merge(hash)`

Merge a hash with another.

```ruby
hash1 = {"a": 1, "b": 2}
hash2 = {"b": 3, "c": 4}
println(hash1.merge(hash2)) # prints: {"a": 1, "b": 3, "c": 4}
```

### `values()`

Returns the list of values from a hash.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println(hash.values()) # prints: [true, 1, ["a", "b", "c"]]
```

## List Builtins

### `clear()`

Clears a list or a hash.

```ruby
list = "Hello".chars() # ["H", "e", "l", "l", "o"]
list.clear() # []
```

### `concat(list)`

Combine two lists into one.

```ruby
println([1,2].concat([3,4])) # prints: [1, 2, 3, 4]
```

### `count(value)`

Count occurrences of a specific value in the list.

```ruby
println("hello world".chars().count("o")) # prints: 2
```

### `dequeue()`

Removes and returns a value from the beginning of a list.

```ruby
list = [1, 2, 3]
println(list.dequeue()) # prints: 1
println(list)           # prints: [2, 3]
```

### `each(lambda)`

Iterate a list, performing some action for each item in the list.

```ruby
# Convert "hello" to a list of unique values, and iterate each.
"hello".chars().unique().each(with (v, i) do
  println("${i} = ${v}")
end)

/# Prints:
0 = h
1 = e
2 = l
3 = o
#/

# Iterate a range.
[1..3].each(with (v, i) do println("${i}: ${v}") end)

/# Prints:
0: 1
1: 2
2: 3
#/

# Iterate a list.
matrix = [[0] * 3] * 3
matrix.each(with (row, row_index) do
  println("${row_index}: ${row}")
end)

/# Prints:
0: [0, 0, 0]
1: [0, 0, 0]
2: [0, 0, 0]
#/
```

### `enqueue(value)`

Pushes a value onto a list.

```ruby
list = [1, 2, 3]
list.enqueue(4)        # prints: [1, 2, 3, 4]
```

### `flatten()`

Flatten nested lists into a single list.

```ruby
println([[1, 2, 3], [[4, 5], 6], [7]].flatten())
# prints: [1, 2, 3, 4, 5, 6, 7]
```

### `first(default_value)`

Returns the first value in a list. Returns `null` if not found.

```ruby
println([1, 2, 3].first()) # prints: 1
```

### `last(default_value)`

Returns the last value in a list. Returns `null` if not found.

```ruby
println([1, 2, 3].first()) # prints: 3
```

### `index(value)`

Returns the index of an item in a list. Returns -1 if not found.

```ruby
println([1, 2, 3, 4, 5].index(1))  # prints: 0
println([1, 2, 3, 4, 5].index(6))  # prints: -1
```

### `insert(value, index)`

Insert a value at a specified index.

```ruby
println([1, 2, 3].insert("a", 2)) # prints: [1, 2, "a", 3]
```

### `join(str)`

Joins a list into a string.

```ruby
println(["Hello", "World!"].join(" ")) # prints: "Hello, World!"
```

### `lastindex(value)`

Returns the last index of an item in a list. Returns -1 if not found.

```ruby
println([1, 0, 0, 1, 0, 1, 1].lastindex(1))  # prints 6
println([1, 2, 3, 4, 5].lastindex(6))        # prints: -1
```

### `map(lambda)`

Transform a list based on a condition.

```ruby
list = ["kiwi", "mango", "banana"]
println(list.map(with (item) do return { "fruit": item, "is_a_kiwi": item.lowercase() == "kiwi" } end))
# prints: [{"fruit": "kiwi", "is_a_kiwi": true}, {"fruit": "mango", "is_a_kiwi": false}, {"fruit": "banana", "is_a_kiwi": false}]
```

### `max()`

Get the highest value in a list.

```ruby
list = [1, 2, 3]
println(list.max()) # prints: 3
```

### `min()`

Get the lowest value in a list.

```ruby
list = [1, 2, 3]
println(list.min()) # prints: 1
```

### `pop()`

Returns and removes a value from the end of a list.

```ruby
list = [1, 2, 3]
println(list.pop()) # prints: 3
println(list)       # prints: [1, 2]
```

### `push(value)`

Pushes a value onto a list.

```ruby
list = [1, 2, 3]
list.push(4)       # prints: [1, 2, 3, 4]
```

### `reduce(accumulator, lambda)`

Aggregate the items in a list.

```ruby
numbers = [1, 2, 3, 4, 5]

sum = numbers.reduce(0, with (accumulator, number) do
    return accumulator + number
end)

println(sum) # prints: 15
```

```ruby
numbers = [1, 2, 3, 4, 5]

hash = numbers.reduce({}, with (accumulator, number) do
    accumulator["key${number}"] = number
    return accumulator
end)

println(hash)
# prints: {"key1": 1, "key2": 2, "key3": 3, "key4": 4, "key5": 5}
```

### `remove(value)`

Remove the first occurrence of a specific value in a list.

```ruby
println([1, 2, 3].remove(2))          # prints: [1, 3]
println(["a", "b", 3, 4].remove("b")) # prints: ["a", 3, 4]
```

### `remove_at(index)`

Remove a value from a list at a specified index.

```ruby
println(["a", "b", 3, 4].remove_at(0)) # prints: ["b", 3, 4]
```

### `reverse()`

Reverse a list or a string.

```ruby
println("kiwi".reverse()) # prints: lartsa
```

### `rotate(n)`

Rotate the values of the list by a specified number of positions. 

If `n` is negative, values are rotated left.

```ruby
println("abcd".chars().rotate(1))  # prints: ["d", "a", "b", "c"]
println("abcd".chars().rotate(0))  # prints: ["a", "b", "c", "d"]
println("abcd".chars().rotate(-1)) # prints: ["b", "c", "d", "a"]
```

### `select(lambda)`

Filter a list based on a condition.

```ruby
list = ["kiwi", "mango", "banana"]
println(list.select(with (item) do return item.contains("s") end))
# prints: ["kiwi"]
```

### `shift()`

Returns the first value of a list.

```ruby
list = [1, 2, 3]
println(list.shift()) # prints: 1
println(list)         # prints: [2, 3]
```

### `size()`

Returns the size of a list or a string as an integer.

```ruby
string = "four"
list = [1, 2, 3, true, false]

println(string.size())
println(list.size())
```

### `slice(start, end)`

Get a subset of the list, specifying start and end indices.

```ruby
println([1, 2, 3].slice(1, 2)) # prints: [2]
println([1, 2, 3].slice(0, 3)) # prints: [1, 2, 3]
println([1, 2, 3].slice(0, 2)) # prints: [1, 2]
```

### `sort()`

Sorting a list.

```ruby
list = ["kiwi", "mango", "guava"]
println(list.sort()) # prints: ["guava", "kiwi", "mango"]
```

### `sum()`

Sum the numeric values in a list.

```ruby
list = [1, 2, 3]
println(list.sum()) # prints: 6
```

### `to_bytes()`

Converts a string or list value to a list of bytes.

```ruby
println("kiwi".to_bytes())         # prints: [97, 115, 116, 114, 97, 108]
println("kiwi".chars().to_bytes()) # prints: [97, 115, 116, 114, 97, 108]
```

### `to_hex()`

Converts a list of integer values to a hexadecimal string.

```ruby
println([97, 115, 116, 114, 97, 108].to_hex()) # prints: 61737472616c
println("kiwi".chars().to_bytes().to_hex())  # prints: 61737472616c
```

### `unique()`

Remove duplicate values from the list.

```ruby
println("aaaabbcccc".chars().unique()) # prints: ["a", "b", "c"]
```

### `unshift(value)`

Inserts a value at the beginning of a list.

```ruby
list = [1, 2, 3]
println(list.unshift(0)) # prints: [0, 1, 2, 3]
println(list)            # prints: [0, 1, 2, 3]
```

### `zip(list)`

Combine values from two lists into pairs.

```ruby
println([1, 2].zip([3, 4])) # prints: [[1, 3], [2, 4]]
```

## Conversion and Type Checking

### `empty()`

Returns true if the value contained is a default value.

```ruby
println((0).empty())    # prints: true
println("".empty())     # prints: true
println([].empty())     # prints: true
println({}.empty())     # prints: true
println(false.empty())  # prints: true
```

### `clone()`

Returns a deep copy of the value.

```ruby
list = [1, 2, 3, true, false]
list2 = list.clone()
list2[0] = "hello"

println(list)  # prints: [1, 2, 3, true, false]
println(list2) # prints: ["hello", 2, 3, true, false]
```

### `is_a(type_name)`

Used for type-checking.

```ruby
println("foobar".is_a(String)) # prints: true
```

### `pretty()`

Returns a pretty serialization.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println(hash.pretty())
/# 
prints:
{
  "key1": true,
  "key2": 1,
  "key3": [
    "a",
    "b",
    "c"
  ]
}
#/
```

### `to_double()`

Converts a numeric value to a double.

```ruby
pi = "3.14159".to_double()
tau = pi * 2
println(tau) # 6.28318
```

### `to_hash()`

Converts an object instance or a JSON string into a hash.

```ruby
class SerializeMe
  def initialize()
    @name = "Kiwi"
  end
end

inst = SerializeMe.new()

println(inst.to_hash()) # prints: {"name": "Kiwi"}
```

```ruby
json = "{
  \"boolean\": true, 
  \"double\": 3.14159, 
  \"integer\": 100, 
  \"string\": \"Kiwi\"
}"
println(json.to_hash()) # prints: {"boolean": true, "double": 3.14159, "integer": 100, "string": "Kiwi"}
```

### `to_int()`

Converts a numeric value to an integer.

```ruby
n = "100".to_int()
n += 0.5
println(n) # 100.5

n = n.to_int()
println(n) # 100
```

### `to_string()`

Converts a value to a string.

```ruby
n = 100
s = [n, n.to_string()]
println(s)  # prints: [100, "100"]
```

### `truthy()`

Returns the truthiness of a value.

```ruby
println null.truthy() # prints: false       # null is never truthy
println (0).truthy()    # prints: false       # 0 is the only non-truthy integer
println (1).truthy()    # prints: true        
println "".truthy()   # prints: false
println "0".truthy()  # prints: true        # non-empty strings are truthy
println [].truthy()   # prints: false
println [0].truthy()  # prints: true        # non-empty lists are truthy
println {}.truthy()   # prints: false       # empty hashes are not truthy
println true.truthy() # prints: true        # true is always truthy
```

### `type()`

Returns the type of the value as a string.

Valid types are: `Integer`, `Double`, `Boolean`, `String`, `List`, `Hash`, `Object`, `Lambda`.

If the type is an object, `type()` will return the class name of the instance.

```ruby
class MyClass
  def initialize()
  end
end

instance = MyClass.new()
println(instance.type()) # prints: MyClass
println("Kiwis are delicious!".type()) # prints: String
```

### `deserialize(str)`

Deserializes a string into a value.

```ruby
string = "[1, 2, 3]"
list = deserialize(string) # Deserialize a string into a list.
list.push(4)               # Push a value to the list.

println(list)              # Prints: [1, 2, 3, 4]
```

### `serialize(value)`

Serializes a value into a string.

```ruby
list = [1, 2, 3]
string = serialize(list) # Serialize a list into a string.

# split the string into a list,
# then print the list of strings in a pretty format.
println(string.chars().pretty())

/# Output:
[
  "[",
  "1",
  ",",
  " ",
  "2",
  ",",
  " ",
  "3",
  "]"
]
#/
```