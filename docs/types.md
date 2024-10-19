# Types

Kiwi supports the following types: 

| Type | Description | Documentation |
| :--- | :--- | :--- |
| [`Integer`](#integer) | A 64-bit integer. | See below for an example. |
| [`Float`](#Float) | A floating point number. | See below for an example. |
| [`Boolean`](#boolean) | A `true` or `false` value. | See below for an example. |
| [`String`](#string) | A sequence of characters. | See [Strings](strings.md). |
| [`List`](#list) | A list of values. | See [Lists](lists.md). |
| [`Hash`](#hash) | A dictionary of key-value pairs. | See [Hashes](hashes.md). |
| [`Object`](#object) | An instance of a `struct`. | See [Structs](structs.md) and [Abstract Structs](abstract_classes.md). |
| [`Lambda`](#lambda) | An anonymous function. | See [lambdas](lambdas.md). |
| [`None`](#none) | A null value. | See below for an example. |

### `Integer`

An 64-bit integer.

```kiwi
# A whole number.
number = 5
number += 5

println(number) # prints: 10
```

### `Float`

A floating point number.

```kiwi
# A real number.
pi = 3.14159
tau = pi * 2

println(tau) # prints: 6.28318
```

### `Boolean`

A `true` or `false` value.

```kiwi
# A value indicating true or false.
enabled = true

println(enabled) # prints: true
```

### `String`

Represents text data.  See [Strings](strings.md).

```kiwi
# A string of text.
hello = "Hello, World!"

println(hello) # prints: Hello, World!
```

### `List`

A list of values.  See [Lists](lists.md).

```kiwi
# A list of values.
list = [1, 2, 3, 4, 5]
println(list) # prints: [1, 2, 3, 4, 5]
```

### `Hash`

A dictionary of key-value pairs.  See [Hashes](hashes.md).

```kiwi
# A simple hash with a single key `language`.
hash = { "language": "kiwi" }

println(hash) # prints: {"language": "kiwi"}
println(hash.language) # prints: kiwi
```

### Object

An instance of a `struct`.  See [Structs](structs.md) and [Abstract Structs](abstract_classes.md).

```kiwi
# Define an empty struct called `MyClass`
struct MyClass
  fn new()
  end
end

# Create an instance of `MyClass`
inst = MyClass.new()
println(inst)
# prints: [Object(struct=MyClass, identifier=inst)]
```

### Lambda

An anonymous function. Lambdas can be assigned identifiers for code reuse.  See [lambdas](lambdas.md).

```kiwi
# Define a reusable lambda called `puts` which accepts a parameter called `s` and prints its value.
puts = with (s) do
  println(s)
end

puts("Hello, World!") # prints: Hello, World!
```

### None

A `null` value. A value that points to nothing.

```kiwi
# Define a hash, `c`, with null values.
c = { "a": null, "b": null }
println(c) # prints: {"a": null, "b": null}

# Deserializing a JSON string with "null" values into a hash.
d = '{"a": null, "b": null}'.to_hash()
println(d) # prints: {"a": null, "b": null}

# Set the hash key values of `a` and `b` to non-null values.
d.a = false
d.b = 'hello'
println(d) # prints: {"a": false, "b": "hello"}

# Set the hash key values of `a` and `b` to null values.
d.a = null
d.b = null
println(d) # prints: {"a": null, "b": null}

# Set `d` to a null value.
d = null
println(d) # prints: null

# Explicitly checking if `d` is null.
if d == null
  println "d is null" # prints: d is null
end

# Set `d` to a non-null value.
d = 0

# Explicitly checking if `d` is non-null.
if d != null
  println "d is not null" # prints: d is not null
end

# Type coercion of null to a Boolean.
if !null
  println "hello world" # prints: hello world
end

```