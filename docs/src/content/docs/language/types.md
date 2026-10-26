---
title: Types
description: Kiwi's built-in types, truthiness, equality, type checks and conversions.
---

Kiwi is dynamically typed: values have types, variables don't. These are the built-in types:

| Type | Description | Example |
| :-- | :-- | :-- |
| `integer` | 64-bit signed integer | `42`, `0xFF`, `0b1010`, `0o17` |
| `float` | 64-bit floating point number | `3.14`, `1.5e3` |
| `boolean` | `true` or `false` | `true` |
| `string` | Unicode text | `"hello"`, `'raw'` |
| `list` | Ordered, mutable sequence of any values | `[1, "two", 3.0]` |
| `hashmap` | Mutable key-value map that keeps insertion order | `{name: "Kiwi"}` |
| `null` | The absence of a value | `null` |
| `bytes` | Byte array | `b"hi"`, `"hi".to_bytes()` |
| `date` | Date and time | `time::now()`, `"2024-06-15".to_date()` |
| `lambda` | A function value: a lambda or a named function | `do (x) => x * 2` |
| `generator` | A lazy sequence returned by a [generator function](/kiwi/language/generators/) | |
| struct instance | An object created from a [struct](/kiwi/language/structs/) | `Point.new(1, 2)` |

## Checking types

`typeof(value)` returns a value's type name as a string. For a struct instance it returns the struct's name; a struct itself is `struct`, and a package value (as returned by `include`) is `package`. The `.type()` method does the same thing.

```kiwi
struct Point
  fn new(x, y)
    @x = x
    @y = y
  end
end

println typeof(42)                 # integer
println typeof(3.14)               # float
println typeof("hi")               # string
println typeof(null)               # null
println typeof(do (x) => x)        # lambda
println typeof(time::now())        # date
println typeof(Point.new(1, 2))    # Point
println typeof(Point)              # struct
println [1, 2].type()              # list
```

`.is_a(type)` tests a value against a type keyword, a type name string, or a struct. For struct instances it also checks parent structs.

```kiwi
struct Animal end
struct Dog < Animal end

println "hi".is_a(string)       # true
println (1).is_a(integer)       # true
println [].is_a("list")         # true
println Dog.new().is_a(Animal)  # true
```

## Truthiness

Conditions accept any value. These values are *falsy*; everything else is *truthy*:

| Type | Falsy value |
| :-- | :-- |
| `boolean` | `false` |
| `null` | `null` |
| `integer` | `0` |
| `float` | `0.0` |
| `string` | `""` |
| `list` | `[]` |
| `hashmap` | `{}` |

`.truthy()` returns a value's truthiness as a boolean:

```kiwi
println (0).truthy()     # false
println (1).truthy()     # true
println "".truthy()      # false
println "0".truthy()     # true
println [].truthy()      # false
println [0].truthy()     # true
println {}.truthy()      # false
println null.truthy()    # false
```

The string `"0"` and the list `[0]` are truthy because they are not empty.

## Equality

`==` compares by value, including lists and hashmaps. Integers and floats compare numerically. Values of different types are otherwise never equal: there is no implicit coercion between strings and numbers, or between `null`, `0` and `false`.

```kiwi
println 1 == 1.0             # true
println [1, 2] == [1, 2]     # true
println {a: 1} == {a: 1}     # true
println "1" == 1             # false
println 0 == false           # false
println null == false        # false
```

## Numbers

Integer arithmetic stays in `integer` except for `/`, which always produces a float. Mixing an integer and a float produces a float. Use `//` for floor division.

```kiwi
println 7 / 2      # 3.5
println 7 // 2     # 3
println 1 + 2.0    # 3
println 0.1 + 0.2  # 0.30000000000000004
```

A float with no fractional part prints without a decimal point (`3.0` prints as `3`), but it is still a float. Use `.to_string("f1")` to control the format.

Integers are 64-bit and wrap around on overflow.

## Conversions

Every value has conversion methods:

| Method | Converts | Example |
| :-- | :-- | :-- |
| `.to_string(format?)` | Any value to a string | `(255).to_string("x")` is `"ff"` |
| `.to_integer()` | Numbers and numeric strings; floats are truncated | `"42".to_integer()` |
| `.to_float()` | Numbers and numeric strings | `"3.5".to_float()` |
| `.to_list()` | A string to characters, bytes to integers | `"abc".to_list()` |
| `.to_bytes()` | A string (UTF-8) or a list of integers to bytes | `"hi".to_bytes()` |
| `.to_date(format?)` | A string or a Unix timestamp in milliseconds to a date | `"2024-06-15".to_date()` |

```kiwi
println "42".to_integer() + 1      # 43
println (3.99).to_integer()        # 3
println (5).to_float() / 2         # 2.5
println (3.14159).to_string("f2")  # 3.14
println "abc".to_list()            # ["a", "b", "c"]
```

Converting a non-numeric string with `.to_integer()` or `.to_float()` throws a `ConversionError`.

`+` with a string on either side converts the other operand to a string:

```kiwi
println "Total: " + 5   # Total: 5
println 5 + "5"         # 55
```

### Number formats

`.to_string(format)` accepts a format code for integers and floats:

| Format | Meaning | Example | Result |
| :-- | :-- | :-- | :-- |
| `"f2"` | Fixed point, 2 decimals (any digit count) | `(5).to_string("f2")` | `5.00` |
| `"x"` / `"X"` | Hexadecimal, lower / upper case | `(255).to_string("X")` | `FF` |
| `"b"` | Binary, padded to 16 digits | `(5).to_string("b")` | `0000000000000101` |
| `"o"` | Octal | `(8).to_string("o")` | `10` |

## Serialization

`serialize(value)` turns a value into its string form, and `deserialize(text)` parses it back. The format is JSON-compatible.

```kiwi
data = {name: "kiwi", tags: ["a", "b"], extra: null}
text = serialize(data)
println text                       # {"name": "kiwi", "tags": ["a", "b"], "extra": null}
println deserialize(text)["tags"]  # ["a", "b"]
```

For JSON specifically, use the [`json`](/kiwi/stdlib/json/) package.

## Bytes

`bytes` values hold raw binary data. Create them with a `b"..."` literal, `.to_bytes()`, or `.hex_bytes()` on a hex string. A `b"..."` literal takes ASCII characters and the escapes `\n`, `\r`, `\t`, `\\`, `\"` and `\xHH` for any byte: `b"hi\x00"` is `[104, 105, 0]`.

```kiwi
data = "kiwi".to_bytes()
println data            # [107, 105, 119, 105]
println data.to_hex()   # 6b697769
println data.size()     # 4
println data.to_string()           # kiwi
println "6b 69".hex_bytes().to_string()   # ki
println b"ki" + b"wi"                      # [107, 105, 119, 105]
```

`+` joins two byte values into new bytes, and `+=` appends.

## Dates

`date` values come from `time::now()`, `.to_date()`, or the global `d()` helper. Their components are available as methods, and the [`date`](/kiwi/stdlib/date/) package adds arithmetic.

```kiwi
release = "2024-06-15".to_date()
println release.year()                      # 2024
println release.month()                     # 6
println release.to_string("yyyy-MM-dd")     # 2024-06-15
println release.add_days(30).to_string("MMM d")   # Jul 15
```

See the [`time`](/kiwi/stdlib/time/) package for clocks, timestamps and formatting.
