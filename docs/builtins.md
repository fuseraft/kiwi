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

Converts a numeric type to a double.

```ruby
@i = 100
```

### `.to_i`

Converts a numeric type to an integer.

```ruby
@n = 100
@n += 0.5
println @n # 100.5

@n = @n.to_i()
println @n # 100
```

### `.to_s`

### `.is_a?`

*Under construction in new interpreter*

Used for type-checking.
