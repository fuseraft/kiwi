# Error Handling

Kiwi provides robust mechanisms for error handling through the use of `try`, `catch`, and `finally` constructs. These constructs allow developers to handle exceptions gracefully, ensuring that the program can recover from errors or at least terminate cleanly.

# Table of Contents
- [Basic Error Handling](#basic-error-handling)
  - [`try` block](#try-block)
  - [`catch` block](#catch-block)
  - [`finally` block](#finally-block)
- [Nested `try-catch` Blocks](#nested-try-catch-blocks)
- [Using `throw` for Custom Errors](#using-throw-for-custom-errors)
- [Error Hashmaps](#error-hashmaps)

## Basic Error Handling

The basic syntax for error handling involves using `try`, `catch`, and `finally` blocks. Here's a simple example to illustrate how they work:

### `try` Block
The `try` block contains the code that might throw an error. If an error occurs, the control is passed to the `catch` block.

### `catch` Block
The `catch` block is used to handle the error. It can capture the error message and take appropriate actions, such as logging the error or performing some recovery steps.

### `finally` Block
The `finally` block contains code that should always be executed, regardless of whether an error occurred or not. This is useful for cleaning up resources.

### Example

```kiwi
fn error_handling_example()
  a = 0, b = 0, c = 0

  try
    a += 1
    println "${1 / 0}"  # This will throw a division by zero error
    a += 1
  catch (err)
    println "Caught an error: ${err}"
    b += 1
  finally
    println "Executing finally block"
    c += 1
  end

  println "Values after error handling: a=${a}, b=${b}, c=${c}"
end

error_handling_example()
```

### Output
```
Caught an error: Attempted to divide by zero.
Executing finally block
Values after error handling: a=1, b=1, c=1
```

## Nested `try-catch` Blocks

Kiwi also supports nested `try-catch` blocks, allowing for more granular error handling within different sections of code.

### Example

```kiwi
fn nested_error_handling_example()
  a = 0, b = 0, c = 0

  for i in [0, 1] do
    try
      a += 1
      println "${i / 0}"  # This will throw a division by zero error
    catch (err)
      println "Caught an error in loop: ${err}"
      b += 1
    finally
      println "Executing finally block in loop"
      c += 1
    end
  end

  println "Values after nested error handling: a=${a}, b=${b}, c=${c}"
end

nested_error_handling_example()
```

### Output
```
Caught an error in loop: Attempted to divide by zero.
Executing finally block in loop
Caught an error in loop: Attempted to divide by zero.
Executing finally block in loop
Values after nested error handling: a=2, b=2, c=2
```

## Using `throw` for Custom Errors

In Kiwi, you can use the `throw` statement to raise custom errors. This can be particularly useful for validating input or enforcing certain conditions within your code.

### Example

```kiwi
fn validate_input(input)
  throw "Invalid input: Must be a positive number" when input <= 0
  println "Valid input: ${input}"
end

fn custom_error_example()
  try
    validate_input(-1)
  catch (err)
    println "Caught an error: ${err}"
  finally
    println "Validation complete"
  end
end

custom_error_example()
```

### Output
```
Caught an error: Invalid input: Must be a positive number
Validation complete
```

## Error Hashmaps

In Kiwi, you can also throw an error using a specialized hashmap containing the following keys:
- `error`: A string containing the error type.
- `message`: A string containing the error message.

This feature enables conditionally handling specific errors when they occur.

```kiwi
# Throwing an error hashmap.
try
  throw { "error": "MyError", "message": "An error occurred!" }
catch (err, msg)
  println("${err}: ${msg}") # prints: MyError: An error occurred!
end
```