# Error Handling 

Kiwi uses the `try-catch` approach for error handling.

## Catching an Error

To capture the error message, use the following syntax: `catch (error_variable_name)`.

```ruby
try
  a = 1
  a /= 0
  println "The line above throws an error."
catch (err)
  println "An error occurred: ${err}" 
  # prints: An error occurred: Attempted to divide by zero.
end
```

## Another Example

Sometimes you just want to catch an error without handling the error message.

```ruby
try
  a = 1
  a /= 0
  println "The line above throws an error."
catch
  println "An error occurred." # prints: An error occurred.
end
```