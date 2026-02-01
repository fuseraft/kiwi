# Lambdas

Lambdas are used to treat callables (i.e. functions) as first-class citizens. You can pass lambdas as method parameters.

# Table of Contents
- [Defining and calling lambdas](#defining-and-calling-lambdas)
  - [Converting a function to a lambda](#converting-a-function-to-a-lambda)
  - [Using `do` (lambda block and arrow functions)](#using-do-lambda-block-and-arrow-functions)
  - [Using the `with` keyword](#using-the-with-keyword)
- [Inline lambdas](#inline-lambdas)
- [Assigned lambdas](#assigned-lambdas)
- [Passing lambdas as parameters](#passing-lambdas-as-parameters)

## Defining and calling lambdas

There are many ways to define a lambda.

### Converting a function to a lambda

You can convert a function to a lambda by calling `to_lambda()` on it.

*Note: Remember you can invoke any callable directly or by calling `.call([parameters])` on it.*

```kiwi
fn say_hello(name: string)
  println "Hello, ${name}!"
end

fn say_bye(name: string)
  println "Bye, ${name}!"
end

fn get_strategy(strategy: string)
  if strategy == "hello"
    return say_hello.to_lambda()
  elsif strategy == "bye"
    return say_bye.to_lambda()
  else
    throw "Unknown strategy"
  end
end

fn use_strategy(strategy: string, name: string)
  get_strategy(strategy).call([name])
end

use_strategy("hello", "World")
use_strategy("bye", "World")
```

### Using `do` (lambda block and arrow functions)

For simple one liners, you might use an arrow function.
```kiwi
println [1 to 10].filter(do (n) => n % 2 == 0)
# [2, 4, 6, 8, 10]
```

Or for more complex requirements, you can implement within a block. 

*Note: Remember, the last evaluation of the callable is its return value!*
```kiwi
println [1 to 10].filter(do (n)
    n % 2 != 0
end)
# [1, 3, 5, 7, 9]
```

Some more examples:
```kiwi
# An assigned lambda
say_hello = do (name: string) => println "Hello, ${name}!"

# Direct invocation of closure
(do (name: string) => println "Hello, ${name}!")("World!")
```

### Using the `with` keyword

For demonstration, we will define a list of hashmaps called `id_list` to be used in the examples below.

```kiwi
# Define a list of hashmaps to work with in our example.
id_list = [
  {"id": 0}, {"id": 1}, {"id": 2}, {"id": 3}, {"id": 4},
  {"id": 5}, {"id": 6}, {"id": 7}, {"id": 8}, {"id": 9}
]
```

## Inline lambdas

Lambdas can be used inline (without assignment).

```kiwi
println(id_list.filter(do (item) => item["id"] % 2 == 0))
# prints: [{"id": 0}, {"id": 2}, {"id": 4}, {"id": 6}, {"id": 8}]
```

## Assigned lambdas

You can assign a reference to a lambda for reuse.

```kiwi
odd_item_id = do (item) => item["id"] % 2 != 0

println(id_list.filter(odd_item_id))
# prints: [{"id": 1}, {"id": 3}, {"id": 5}, {"id": 7}, {"id": 9}]
```

## Passing lambdas as parameters

You can pass lambdas as parameters to methods.

```kiwi
puts = do (s) => println(s)

puts("Hello, World!") # prints: Hello, World!

fn use_lambda(print_func, message)
  print_func(message)
end

use_lambda(puts, "Hello, Kiwi!") # prints: Hello, Kiwi!
```
