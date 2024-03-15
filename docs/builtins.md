# Builtins

In Kiwi, builtins are accessed using dot-notation and can be used to query or manipulate values and types.

Currently, Kiwi supports the following builtins:

## Table of Contents
- [`begins_with(str)`](#begins_withstr)
- [`chars()`](#chars)
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
- [`reduce(accumulator, lambda)`](#reduceaccumulator-lambda)
- [`replace(search, replacement)`](#replacesearch-replacement)
- [`reverse()`](#reverse)
- [`rtrim()`](#rtrim)
- [`select(lambda)`](#selectlambda)
- [`size()`](#size)
- [`sort()`](#sort)
- [`split(delim)`](#splitdelim)
- [`sum()`](#sum)
- [`to_d()`](#tod)
- [`to_h()`](#toh)
- [`to_i()`](#toi)
- [`to_s()`](#tos)
- [`trim()`](#trim)
- [`type()`](#type)
- [`upcase(str)`](#upcasestr)

### `chars()`

Converts a string into a list. Each character in the string becomes a new string in the list.

```ruby
string = "Hello"

chars = string.chars() 
# chars = ["H", "e", "l", "l", "o"]

fruit = "Kiwi".chars()
# fruit = ["K", "i", "w", "i"]
```

### `join(str)`

Joins a list into a string.

```ruby
println ["Hello", "World!"].join(" ") # prints: "Hello, World!"
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
println "kiwi".reverse() # prints: iwik
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

### `to_d()`

Converts a numeric value to a double.

```ruby
pi = "3.14159".to_d()
tau = pi * 2
println tau # 6.28318
```

### `to_i()`

Converts a numeric value to an integer.

```ruby
n = "100".to_i()
n += 0.5
println n # 100.5

n = n.to_i()
println n # 100
```

### `to_s()`

Converts a value to a string.

```ruby
n = 100
s = [n, n.to_s()]
println s  # prints: [100, "100"]
```

### `to_h()`

Converts an object instance or a JSON string into a hash.

```ruby
class SerializeMe
  def initialize()
    this.name = "Kiwi"
  end
end

inst = SerializeMe.new()

println inst.to_h() # prints: {"name": "Kiwi"}
```

```ruby
json = "{
  \"boolean\": true, 
  \"double\": 3.14159, 
  \"integer\": 100, 
  \"string\": \"Kiwi\"
}"
println json.to_h() # prints: {"boolean": true, "double": 3.14159, "integer": 100, "string": "Kiwi"}
```

### `index(value)`

Returns the index of a string. Returns -1 if not found.

```ruby
println "foobar".index("bar")  # prints: 3
println "foobar".index("kiwi")  # prints: -1
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
println "foobar".lastindex("kiwi")  # prints: -1
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
list = ["kiwi", "mango", "guava"]
println list.sort() # prints: ["guava", "kiwi", "mango"]
```

### `select(lambda)`

Filter a list based on a condition.

```ruby
list = ["kiwi", "mango", "banana"]
println list.select(lambda (item) do return item.contains("i") end)
# prints: ["kiwi"]
```

### `map(lambda)`

Transform a list based on a condition.

```ruby
list = ["kiwi", "mango", "banana"]
println list.map(lambda (item) do return { "fruit": item, "is_a_kiwi": item.downcase() == "kiwi" } end)
# prints: [{"fruit": kiwi, "is_a_kiwi": true}, {"fruit": mango, "is_a_kiwi": false}, {"fruit": banana, "is_a_kiwi": false}]
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