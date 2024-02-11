# Builtins

In Kiwi, builtins are accessed using dot-notation and can be used to query or manipulate values and types.

Currently, Kiwi supports the following builtins:

### `.chars()`

Converts a string into a list. Each character in the string becomes a new string in the list.

```ruby
@string = "Hello"

@chars = @string.chars() 
# @chars = ["H", "e", "l", "l", "o"]

@fruit = "Kiwi".chars()
# @fruit = ["K", "i", "w", "i"]
```

### `.join(str)`

Joins a list into a string.

```ruby
println ["Hello", "World!"].join(" ") # prints: "Hello, World!"
```

### `.split(delim)`

Splits a string into a list by delimiter.

```ruby
println "Hello World!".split(" ") # prints: ["Hello", "World!"]
```

### `.size()`

Returns the size of a list or a string as an integer.

```ruby
@string = "four"
@list = [1, 2, 3, true, false]

println @string.size()
println @list.size()
```

### `.ltrim()`

Trims whitespace from the left-hand side of a string.

```ruby
println "   Hello World!".ltrim() # prints: Hello World!
```

### `.rtrim()`

Trims whitespace from the right-hand side of a string.

```ruby
println "Hello World!    ".rtrim() + " Testing!" # prints: Hello World! Testing!
```

### `.rtrim()`

Trims whitespace from both sides of a string.

```ruby
println "     Hello World!    ".trim() + " Testing!" # prints: Hello World! Testing!
```

### `.type()`

Returns the type of the value as a string.

Valid types are: `Integer`, `Double`, `Boolean`, `String`, `List`, `Hash`, `Object`.

If the type is an object, `type()` will return the class name of the instance.

```ruby
class MyClass
  def initialize()
  end
end

@instance = MyClass.new()
println @instance.type() # prints: MyClass
println "Kiwis are delicious!".type() # prints: String
```

### `.to_d()`

Converts a numeric value to a double.

```ruby
@pi = "3.14159".to_d()
@tau = @pi * 2
println @tau # 6.28318
```

### `.to_i()`

Converts a numeric value to an integer.

```ruby
@n = "100".to_i()
@n += 0.5
println @n # 100.5

@n = @n.to_i()
println @n # 100
```

### `.to_s()`

Converts a value to a string.

```ruby
@n = 100
@s = [@n, @n.to_s()]
println @s  # prints: [100, "100"]
```

### `.index_of(str)`

Returns the index of a string.

```ruby
println "foobar".index_of("bar")  # prints: 3
```

### `.begins_with(str)`

Returns true if the string begins with a given string.

```ruby
println "foobar".begins_with("foo")   # prints: true
println "foobar".begins_with("food")  # prints: false
```

### `.contains(str)`

Returns true if the string contains a given string.

```ruby
println "foobar".contains("bar")   # prints: true
println "foobar".contains("bark")  # prints: false
```

### `.ends_with(str)`

Returns true if the string ends with a given string.

```ruby
println "foobar".ends_with("bar")   # prints: true
println "foobar".ends_with("bark")  # prints: false
```

### `.upcase(str)`

Returns the uppercase value of a string.

```ruby
println "foobar".upcase()   # prints: FOOBAR
```

### `.downcase(str)`

Returns the lowercase value of a string.

```ruby
println "FOOBAR".downcase()   # prints: foobar
```

### `.replace(search, replacement)`

Search for a string and replace with a given string.

```ruby
println "foobar".replace("foo", "food")   # prints: foodbar
```

### `.is_a(type_name)`

Used for type-checking.

```ruby
println "foobar".is_a(String) # prints: true
```

### `.keys()`

Returns a list of keys from a hash.

```ruby
@hash = {
  "key1": true, 
  "key2": 1, 
  "key3": ["a", "b", "c"]
}

println @hash.keys() # prints: ["key1", "key2", "key3"]
```

### `.select(lambda)`

Filter a list based on a condition.

```ruby
@list = ["kiwi", "mango", "banana"]
println @list.select(lambda (@item) do return @item.contains("i") end)
# prints: ["kiwi"]
```

### `.map(lambda)`

Transform a list based on a condition.

```ruby
@list = ["kiwi", "mango", "banana"]
println @list.map(lambda (@item) do return { "fruit": @item, "is_a_kiwi": @item.downcase() == "kiwi" } end)
# prints: [{"fruit": kiwi, "is_a_kiwi": true}, {"fruit": mango, "is_a_kiwi": false}, {"fruit": banana, "is_a_kiwi": false}]
```

### `.reduce(accumulator, lambda)`

```ruby
@numbers = [1, 2, 3, 4, 5]

@sum = @numbers.reduce(0, lambda (@accumulator, @number) do
    return @accumulator + @number
end)

println @sum # prints: 15
```