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
println ["Hello", "World!"].join(" ")
# Prints "Hello, World!"
```

### `.size()`

Returns the size of a list or a string as an integer.

```ruby
@string = "four"
@list = [1, 2, 3, true, false]

println @string.size()
println @list.size()
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

### `.is_a?(type_name)`

*Under construction in new interpreter*

Used for type-checking.
