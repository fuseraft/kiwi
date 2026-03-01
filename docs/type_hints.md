# Type Hints

Kiwi supports type hints for function arguments and return values. Type hints help ensure code correctness and improve readability by allowing the programmer to specify expected types for function inputs and outputs. Currently, Kiwi performs runtime type checking based on these hints.

## Syntax

Type hints in Kiwi are specified by adding a colon (`:`) followed by the expected type after the parameter or return value in a function declaration.

```kiwi
fn function_name(parameter: type, ...): return_type
  # function body
end
```

### Example

```kiwi
fn add_numbers(a: integer, b: integer): integer
  return a + b
end
```

In this example, both `a` and `b` are expected to be of type `integer`, and the function is expected to return an `integer`.

## Supported Types

Kiwi’s type hint system supports the following builtin types:

- **integer**: Represents integer numbers.
- **float**: Represents floating-point numbers.
- **string**: Represents sequences of characters.
- **boolean**: Represents a truth value (`true` or `false`).
- **list**: Represents a list of items.
- **hashmap**: Represents a key-value map.
- **object**: Represents an object.
- **lambda**: Represents a lambda.
- **bytes**: Represents a byte array.
- **pointer**: Represents a pointer to a managed reference.
- **generator**: Represents a generator object (returned by a generator function).
- **none**: Represents a `null` value.
- **any**: Accepts any type. Useful for parameters or return values where the type is flexible.

## Function Parameters with Type Hints

To add type hints to function parameters, specify the parameter name followed by `: Type`. Kiwi checks these types at runtime, raising an error if the arguments passed do not match the expected types.

### Example

```kiwi
fn concatenate_strings(prefix: string, suffix: string): string
  prefix + suffix
end

println concatenate_strings("Hello, ", "World!") # Output: Hello, World!
```

If the arguments passed to `concatenate_strings` are not `string` types, Kiwi will raise a type error.

## Union Types

A parameter or return type can accept more than one type by separating types with `|`. Kiwi accepts a value if it matches **any** of the listed types.

### Syntax

```kiwi
fn function_name(parameter: type1|type2|...): return_type1|return_type2|...
  # function body
end
```

### Nullable Parameters

The most common use case is allowing `null` alongside a concrete type:

```kiwi
fn greet(name: string|none = null)
  if name == null
    println("Hello, stranger!")
  else
    println("Hello, ${name}!")
  end
end

greet()          # Hello, stranger!
greet("Scotty")  # Hello, Scotty!
```

### Multiple Types

```kiwi
fn process(val: integer|float = 0)
  println("value: ${val}")
end

process()      # value: 0
process(42)    # value: 42
process(3.14)  # value: 3.14
```

### Union Return Types

A function can declare multiple valid return types:

```kiwi
fn divide(a: integer, b: integer): integer|float
  if b == 0
    return 0
  end
  return a / b
end

fn maybe(flag: boolean): string|none
  if flag
    return "hello"
  end
  return null
end
```

### Type Mismatch

If a value matches none of the listed types, Kiwi raises a type error showing the full union. This applies to both parameters and return values:

```kiwi
fn strict(x: integer|string = 0)
  println(x)
end

strict(true)  # Error: Expected type `integer|string` for parameter 1 of `strict` but received `boolean`.

fn bad(): integer|float
  return "oops"
end

bad()  # Error: Expected type `integer|float` for return type of `bad` but received `string`.
```

## Default Values with Type Hints

Type hints can be used with parameters that have default values.

### Example

```kiwi
fn greet(name: string, greeting: string = "Hello"): string
  greeting + ", " + name
end

println greet("Alice")           # Output: Hello, Alice
println greet("Alice", "Hi")     # Output: Hi, Alice
```

## Type Hints for Return Types

The return type hint specifies the expected type of the function’s result.

### Example

```kiwi
fn is_even(number: integer): boolean
  return number % 2 == 0
end
```

In this example, the function `is_even` is expected to return a `boolean` value.

## Type Checking at Runtime

Kiwi performs runtime type checking for all parameters with type hints. If an argument does not match the specified type, Kiwi raises an error and halts the function’s execution.

### Example with Error Handling

```kiwi
fn multiply(a: integer, b: integer): integer
  return a * b
end

multiply(5, "10")  # Error: Expected `integer` for parameter 2 of `multiply`, but received `string`.
```

In this example, an error is raised because `b` is expected to be an `integer`, but a `string` was provided.