# Types

Astral supports working with the following types: 

| Type | Description | Documentation |
| :--- | :--- | :--- |
| `Integer` | An integer. | See below for an example. |
| `Double` | A floating point number. | See below for an example. |
| `Boolean` | A `true` or `false` value. | See below for an example. |
| `String` | A sequence of characters. | See below for an example. |
| `List` | A list of values. | See [lists.md](lists.md) |
| `Hash` | A dictionary of key-value pairs. | See [hashes.md](hashes.md) |
| `Object` | An instance of a `class`. | See [classes.md](classes.md) and [abstract_classes.md](abstract_classes.md) |
| `Lambda` | An anonymous function. | See [lambdas.md](lambdas.md) |

### Integer

An integer.

```ruby
number = 5
number += 5

println number # prints: 10
```

### Double

A floating point number.

```ruby
pi = 3.14159
tau = pi * 2

println tau # prints: 6.28318
```

### Boolean

A `true` or `false` value.

```ruby
enabled = true

println enabled # prints: true
```

### String

A sequence of characters.

```ruby
string = "Hello, World!"

println string # prints: Hello, World!
```

### List

A list of values.

```ruby
list = [1, 2, 3, 4, 5]
println list # prints: [1, 2, 3, 4, 5]
```

### Hash

A dictionary of key-value pairs.

```ruby
hash = { "language": "astral" }

println hash  # prints: {"language": "astral"}
```

### Object

An instance of a `class`.

```ruby
class MyClass
  def initialize()
  end
end

inst = MyClass.new()
println inst
# prints: [Object(class=MyClass, identifier=inst)]
```

### Lambda

An anonymous function. Lambdas can be assigned identifiers for code reuse.

```ruby
puts = with (s) do
  println s
end

puts("Hello, World!") # prints: Hello, World!
```

