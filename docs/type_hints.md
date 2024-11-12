# Type Hints

Kiwi supports type hints for function arguments and return values. Type hints help ensure code correctness and improve readability by allowing the programmer to specify expected types for function inputs and outputs. Currently, Kiwi performs runtime type checking based on these hints.

## Syntax

Type hints in Kiwi are specified by adding a colon (`:`) followed by the expected type after the parameter or return value in a function declaration.

```kiwi
fn function_name(parameter: Type, ...): ReturnType
  # function body
end
```

### Example

```kiwi
fn add_numbers(a: Integer, b: Integer): Integer
  return a + b
end
```

In this example, both `a` and `b` are expected to be of type `Integer`, and the function is expected to return an `Integer`.

## Supported Types

Kiwi’s type hint system supports the following types:

- **Integer**: Represents integer numbers.
- **Float**: Represents floating-point numbers.
- **String**: Represents sequences of characters.
- **Boolean**: Represents a truth value (`true` or `false`).
- **List**: Represents a list of items.
- **Hash**: Represents a key-value map.
- **Function**: Represents a callable function.
- **Object**: Represents an object.
- **Lambda**: Represents a lambda.
- **Any**: Accepts any type. Useful for parameters or return values where the type is flexible.

## Function Parameters with Type Hints

To add type hints to function parameters, specify the parameter name followed by `: Type`. Kiwi checks these types at runtime, raising an error if the arguments passed do not match the expected types.

### Example

```kiwi
fn concatenate_strings(prefix: String, suffix: String): String
  return prefix + suffix
end

println concatenate_strings("Hello, ", "World!") # Output: Hello, World!
```

If the arguments passed to `concatenate_strings` are not `String` types, Kiwi will raise a type error.

## Default Values with Type Hints

Type hints can be used with parameters that have default values.

### Example

```kiwi
fn greet(name: String, greeting: String = "Hello"): String
  return greeting + ", " + name
end

println greet("Alice")           # Output: Hello, Alice
println greet("Alice", "Hi")      # Output: Hi, Alice
```

## Type Hints for Return Types

The return type hint specifies the expected type of the function’s result.

### Example

```kiwi
fn is_even(number: Integer): Boolean
  return number % 2 == 0
end
```

In this example, the function `is_even` is expected to return a `Boolean` value.

## Type Checking at Runtime

Kiwi performs runtime type checking for all parameters with type hints. If an argument does not match the specified type, Kiwi raises an error and halts the function’s execution.

### Example with Error Handling

```kiwi
fn multiply(a: Integer, b: Integer): Integer
  return a * b
end

multiply(5, "10")  # Error: Expected `Integer` for parameter 2 of `multiply`, but received `String`.
```

In this example, an error is raised because `b` is expected to be an `Integer`, but a `String` was provided.