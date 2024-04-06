# Builtins

In Astral, builtins are accessed using dot-notation and can be used to query or manipulate values and types.

Currently, Astral supports the following builtins:

## Table of Contents
- [`begins_with(str)`](#begins_withstr)
- [`chars()`](#chars)
- [`clear()`](#clear)
- [`contains(str)`](#containsstr)
- [`downcase(str)`](#downcasestr)
- [`ends_with(str)`](#ends_withstr)
- [`index(value)`](#indexvalue)
- [`is_a(type_name)`](#is_atype_name)
- [`join(str)`](#joinstr)
- [`keys()`](#keys)
- [`lastindex(value)`](#lastindexvalue)
- [`ltrim()`](#ltrim)
- [`map(lambda)`](#maplambda)
- [`max()`](#max)
- [`min()`](#min)
- [`pop()`](#pop)
- [`push(value)`](#pushvalue)
- [`dequeue()`](#dequeue)
- [`enqueue(value)`](#enqueuevalue)
- [`reduce(accumulator, lambda)`](#reduceaccumulator-lambda)
- [`replace(search, replacement)`](#replacesearch-replacement)
- [`reverse()`](#reverse)
- [`rtrim()`](#rtrim)
- [`select(lambda)`](#selectlambda)
- [`size()`](#size)
- [`sort()`](#sort)
- [`split(delim)`](#splitdelim)
- [`substring(pos, length)`](#substringpos-length)
- [`sum()`](#sum)
- [`to_bytes()`](#to_bytes)
- [`to_hex()`](#to_hex)
- [`to_double()`](#to_double)
- [`to_hash()`](#to_hash)
- [`to_int()`](#to_int)
- [`to_string()`](#to_string)
- [`trim()`](#trim)
- [`type()`](#type)
- [`upcase(str)`](#upcasestr)

### `chars()`

Converts a string into a list. Each character in the string becomes a new string in the list.

```ruby
string = "Hello"

chars = string.chars() 
# chars = ["H", "e", "l", "l", "o"]

println "astral".chars() # prints: ["a", "s", "t", "r", "a", "l"]
```

### `clear()`

Clears a list or a hash.

```ruby
list = "Hello".chars() # ["H", "e", "l", "l", "o"]
list.clear() # []
```
### `join(str)`

Joins a list into a string.

```ruby
println ["Hello", "World!"].join(" ") # prints: "Hello, World!"
```

### `substring(pos, length)`

Extract a substring from a string.

```ruby
println "hello".substring(1)    # prints: ello
println "hello".substring(1, 2) # prints: el
```

### `split(delim)`

Splits a string into a list by delimiter.

```ruby
println "Hello World!".split(" ") # prints: ["Hello", "World!"]
```

### `size()`

Returns the size of a list or a string as an integer.

```ruby
string = "four"
list = [1, 2, 3, true, false]

println string.size()
println list.size()
```

### `reverse()`

Reverse a list or a string.

```ruby
println "astral".reverse() # prints: lartsa
```

### `ltrim()`

Trims whitespace from the left-hand side of a string.

```ruby
println "   Hello World!".ltrim() # prints: Hello World!
```

### `rtrim()`

Trims whitespace from the right-hand side of a string.

```ruby
println "Hello World!    ".rtrim() + " Testing!" # prints: Hello World! Testing!
```

### `trim()`

Trims whitespace from both sides of a string.

```ruby
println "     Hello World!    ".trim() + " Testing!" # prints: Hello World! Testing!
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
println instance.type() # prints: MyClass
println "Kiwis are delicious!".type() # prints: String
```

### `to_bytes()`

Converts a `String` or `List` value to a list of bytes.

```ruby
println "astral".to_bytes()         # prints: [97, 115, 116, 114, 97, 108]
println "astral".chars().to_bytes() # prints: [97, 115, 116, 114, 97, 108]
```

### `to_hex()`

Converts a `List` of `Integer` values to a hexadecimal string.

```ruby
println [97, 115, 116, 114, 97, 108].to_hex() # prints: 61737472616c
println "astral".chars().to_bytes().to_hex()  # prints: 61737472616c
```

### `to_double()`

Converts a numeric value to a double.

```ruby
pi = "3.14159".to_double()
tau = pi * 2
println tau # 6.28318
```

### `to_int()`

Converts a numeric value to an integer.

```ruby
n = "100".to_int()
n += 0.5
println n # 100.5

n = n.to_int()
println n # 100
```

### `to_string()`

Converts a value to a string.

```ruby
n = 100
s = [n, n.to_string()]
println s  # prints: [100, "100"]
```

### `to_hash()`

Converts an object instance or a JSON string into a hash.

```ruby
class SerializeMe
  def initialize()
    this.name = "Astral"
  end
end

inst = SerializeMe.new()

println inst.to_hash() # prints: {"name": "Astral"}
```

```ruby
json = "{
  \"boolean\": true, 
  \"double\": 3.14159, 
  \"integer\": 100, 
  \"string\": \"Astral\"
}"
println json.to_hash() # prints: {"boolean": true, "double": 3.14159, "integer": 100, "string": "Astral"}
```

### `index(value)`

Returns the index of a string. Returns -1 if not found.

```ruby
println "foobar".index("bar")  # prints: 3
println "foobar".index("astral")  # prints: -1
```

Returns the index of an item in a list. Returns -1 if not found.

```ruby
println [1, 2, 3, 4, 5].index(1)  # prints: 0
println [1, 2, 3, 4, 5].index(6)  # prints: -1
```

### `lastindex(value)`

Returns the last index of a string. Returns -1 if not found.

```ruby
println "foobarbar".lastindex("bar")  # prints: 6
println "foobar".lastindex("astral")  # prints: -1
```

Returns the last index of an item in a list. Returns -1 if not found.

```ruby
println [1, 0, 0, 1, 0, 1, 1].lastindex(1)  # prints 6
println [1, 2, 3, 4, 5].lastindex(6)        # prints: -1
```

### `begins_with(str)`

Returns true if the string begins with a given string.

```ruby
println "foobar".begins_with("foo")   # prints: true
println "foobar".begins_with("food")  # prints: false
```

### `contains(str)`

Returns true if the string contains a given string.

```ruby
println "foobar".contains("bar")   # prints: true
println "foobar".contains("bark")  # prints: false
```

### `ends_with(str)`

Returns true if the string ends with a given string.

```ruby
println "foobar".ends_with("bar")   # prints: true
println "foobar".ends_with("bark")  # prints: false
```

### `upcase(str)`

Returns the uppercase value of a string.

```ruby
println "foobar".upcase()   # prints: FOOBAR
```

### `downcase(str)`

Returns the lowercase value of a string.

```ruby
println "FOOBAR".downcase()   # prints: foobar
```

### `pop()`

Returns and removes a value from the end of a list.

```
list = [1, 2, 3]
println list.pop() # prints: 3
println list       # prints: [1, 2]
```

### `push(value)`

Pushes a value onto a list.

```
list = [1, 2, 3]
list.push(4)       # prints: [1, 2, 3, 4]
```

### `dequeue()`

Removes and returns a value from the beginning of a list.

```
list = [1, 2, 3]
println list.dequeue() # prints: 1
println list           # prints: [2, 3]
```

### `enqueue(value)`

Pushes a value onto a list.

```
list = [1, 2, 3]
list.enqueue(4)        # prints: [1, 2, 3, 4]
```

### `replace(search, replacement)`

Search for a string and replace with a given string.

```ruby
println "foobar".replace("foo", "food")   # prints: foodbar
```

### `is_a(type_name)`

Used for type-checking.

```ruby
println "foobar".is_a(String) # prints: true
```

### `keys()`

Returns a list of keys from a hash.

```ruby
hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println hash.keys() # prints: ["key1", "key2", "key3"]
```

### `min()`

Get the lowest value in a list.

```
list = [1, 2, 3]
println list.min() # prints: 1
```

### `max()`

Get the highest value in a list.

```
list = [1, 2, 3]
println list.max() # prints: 3
```

### `sum()`

Sum the numeric values in a list.

```
list = [1, 2, 3]
println list.sum() # prints: 6
```

### `sort()`

Sorting a list.

```
list = ["astral", "mango", "guava"]
println list.sort() # prints: ["guava", "astral", "mango"]
```

### `select(lambda)`

Filter a list based on a condition.

```ruby
list = ["astral", "mango", "banana"]
println list.select(lambda (item) do return item.contains("i") end)
# prints: ["astral"]
```

### `map(lambda)`

Transform a list based on a condition.

```ruby
list = ["kiwi", "mango", "banana"]
println list.map(lambda (item) do return { "fruit": item, "is_a_kiwi": item.downcase() == "kiwi" } end)
# prints: [{"fruit": "kiwi", "is_a_kiwi": true}, {"fruit": "mango", "is_a_kiwi": false}, {"fruit": "banana", "is_a_kiwi": false}]
```

### `reduce(accumulator, lambda)`

```ruby
numbers = [1, 2, 3, 4, 5]

sum = numbers.reduce(0, lambda (accumulator, number) do
    return accumulator + number
end)

println sum # prints: 15
```

```ruby
numbers = [1, 2, 3, 4, 5]

hash = numbers.reduce({}, lambda (accumulator, number) do
    accumulator["key${number}"] = number
    return accumulator
end)

println hash 
# prints: {"key1": 1, "key2": 2, "key3": 3, "key4": 4, "key5": 5}
```
