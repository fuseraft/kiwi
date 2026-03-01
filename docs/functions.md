# Functions and Methods

Use the `fn` keyword to define a function or a method (a function of a [`struct`](structs.md)). 

```kiwi
fn greet(name = "Kiwi")
  println("Hello, ${name}!")
end

greet("world") # prints: Hello, world!
greet()        # prints: Hello, Kiwi!
```

## Table of Contents

- [Return Value](#return-value)
- [Optional Parameters](#optional-parameters)
- [Named Parameters](#named-parameters)
- [Scope](#scope)
- [Generator Functions](#generator-functions)

### Return Value

Use the `return` keyword to return a value from a method, or to exit a method early.

```kiwi
fn get_greeting(name)
  return "Hello, ${name}"
end

greeting = get_greeting("World!")

println(greeting)
```

### Optional Parameters

```kiwi
fn say(msg = "Hello, World!")
  println(msg)
end

say()       # prints: Hello, World!
say("Hey!") # prints: Hey!

fn configure(data, config = {})
  for key in config.keys() do
    data[key] = config[key]
  end

  return data
end

data = configure({ "name": "Scott" })
println(data) # prints: {"name": "Scott"}

data = configure({ "name": "Scott" }, { "favorite_os": "Fedora" })
println(data) # prints: {"name": "Scott", "favorite_os": "Fedora"}
```

### Named Parameters

Arguments can be passed by name, in any order. Named and positional arguments can be mixed — positional arguments fill the first available unfilled slot from left to right.

```kiwi
fn greet(name: string, greeting: string = "Hello")
  println "${greeting}, ${name}!"
end

greet("Scotty")                     # Hello, Scotty!
greet(name="Scotty")                # Hello, Scotty!
greet(greeting="Hi", name="Scotty") # Hi, Scotty!
greet("Scotty", greeting="Hey")     # Hey, Scotty!
greet(greeting: "Hola", name: "Scotty") # Hola, Scotty!
```

Passing an unknown parameter name or specifying the same parameter more than once is an error.

```kiwi
fn add(a: integer, b: integer): integer
  return a + b
end

println add(b=3, a=7) # 10
```

### Scope

You can access all global variables from within a method.

```kiwi
counter = 0

fn uptick()
  counter += 1
end

i = 1
while i <= 5 do
  uptick()
  i += 1
end

# 5
println(counter)
```

### Generator Functions

A function that contains `yield` is a **generator function**. Calling it returns a lazy generator object instead of executing the body. See [Generators](generators.md) for full details.

```kiwi
fn countdown(n)
  while n >= 0 do
    yield n
    n -= 1
  end
end

for x in countdown(5) do
  print "${x} "
end
# 5 4 3 2 1 0
```
