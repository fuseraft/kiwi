# Error Handling 

Astral uses the `try-catch` approach for error handling.

## Throwing an Error

To throw an error, use the `throw` keyword.

```ruby
# Throwing an error hash.
try
  throw { "error": "MyError", "message": "An error occurred!" }
catch (err, msg)
  println "${err}: ${msg}" # prints: MyError: An error occurred!
end

# Throwing an error with a message only.
try
  throw "An error occurred!"
catch (err, msg)
  println "${err}: ${msg}" # prints: AstralError: An error occurred!
end
```

## Catching an Error

To capture the error type and the message, use the following syntax: `catch (error_variable, message_variable)`.

```ruby
try
  a = 1
  a /= 0
  println "The line above throws an error."
catch (err, msg)
  println "${err}: ${msg}" # prints: DivideByZeroError: Attempted to divide by zero.
end
```

To capture just the error message, use the following syntax: `catch (message_variable)`.

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

## Catching Any Error

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