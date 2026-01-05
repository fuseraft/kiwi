# Types

Kiwi supports the following builtin types. To build your own type, see [Structs](structs.md).

| Type | Description | Documentation |
| :--- | :--- | :--- |
| [`boolean`](#boolean) | A `true` or `false` value. | See below for an example. |
| [`bytes`](#bytes) | A byte array. | See below for an example. |
| [`date`](#date) | A date-time object. | See [Dates](dates.md). |
| [`float`](#Float) | A floating point number. | See below for an example. |
| [`hashmap`](#hashmap) | A dictionary of key-value pairs. | See [Hashmaps](hashmaps.md). |
| [`integer`](#integer) | A 64-bit integer. | See below for an example. |
| [`lambda`](#lambda) | An anonymous function. | See [lambdas](lambdas.md). |
| [`list`](#list) | A list of values. | See [Lists](lists.md). |
| [`none`](#none) | A null value. | See below for an example. |
| [`object`](#object) | An instance of a `struct`. | See [Structs](structs.md) and [Abstract Structs](abstract_structs.md). |
| [`pointer`](#pointer) | A pointer to a managed reference. | See example below. |
| [`string`](#string) | A sequence of characters. | See [Strings](strings.md). |

---

### `boolean`

A `true` or `false` value.

```kiwi
# A value indicating true or false.
enabled = true

println(enabled) # prints: true
```

---

### `bytes`

A byte array.

```kiwi
ascii_chars = [33..126].to_bytes()

println ascii_chars.to_string() 

/# prints:
!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
#/
```

---

### `date`

Represents a date and time.  See [Dates](dates.md).

```kiwi
dt = "2023-01-02".to_date()

println(dt) # prints: 1/2/2023 12:00:00 AM
```

---

### `float`

A floating point number.

```kiwi
# A real number.
pi = 3.14159
tau = pi * 2

println(tau) # prints: 6.28318
```

---

### `hashmap`

A dictionary of key-value pairs.  See [Hashmaps](hashmaps.md).

```kiwi
# A simple hashmap with a single key `language`.
hashmap = { "language": "kiwi" }

println(hashmap) # prints: {"language": "kiwi"}
println(hashmap.language) # prints: kiwi
```

---

### `integer`

An 64-bit integer.

```kiwi
# A whole number.
number = 5
number += 5

println(number) # prints: 10
```

---

### `lambda`

An anonymous function. Lambdas can be assigned identifiers for code reuse.  See [lambdas](lambdas.md).

```kiwi
# Define a reusable lambda called `puts` which accepts a parameter called `s` and prints its value.
puts = with (s) do
  println(s)
end

puts("Hello, World!") # prints: Hello, World!
```

---

### `list`

A list of values.  See [Lists](lists.md).

```kiwi
# A list of values.
list = [1, 2, 3, 4, 5]
println(list) # prints: [1, 2, 3, 4, 5]
```

---

### `none`

A `null` value. A value that points to nothing.

```kiwi
# Define a hashmap, `c`, with null values.
c = { "a": null, "b": null }
println(c) # prints: {"a": null, "b": null}

# Deserializing a JSON string with "null" values into a hashmap.
d = '{"a": null, "b": null}'.to_hashmap()
println(d) # prints: {"a": null, "b": null}

# Set the hashmap key values of `a` and `b` to non-null values.
d.a = false
d.b = 'hello'
println(d) # prints: {"a": false, "b": "hello"}

# Set the hashmap key values of `a` and `b` to null values.
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

---

### `object`

An instance of a `struct`.  See [Structs](structs.md) and [Abstract Structs](abstract_structs.md).

```kiwi
# Define an empty struct called `MyStruct`
struct MyStruct end

# Create an instance of `MyStruct`
inst = MyStruct.new()
println(inst)
# prints: [Object(struct=MyStruct, identifier=inst)]
```
---

### `pointer`

A pointer to a managed reference. For internal use only.

```kiwi
cap = 3
chan = __chan_create__(cap)
__chan_send__(chan, "hello")
println __chan_recv__(chan) # prints hello
__chan_close__(chan)
```

---

### `string`

Represents text data.  See [Strings](strings.md).

```kiwi
# A string of text.
hello = "Hello, World!"

println(hello) # prints: Hello, World!
```