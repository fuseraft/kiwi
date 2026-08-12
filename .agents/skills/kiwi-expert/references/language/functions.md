# Functions and Methods

Use the `fn` keyword to define a function or a method (a function of a [`struct`](structs.md)). 

```kiwi
fn greet(name = "Kiwi")
  println("Hello, ${name}!")
end

greet("world") # prints: Hello, world!
greet()        # prints: Hello, Kiwi!
```

### Return Value

A function returns the value of its **last evaluated expression** — `return` is optional. Use `return` when you need to return early or when the expression isn't naturally last.

```kiwi
fn add(a, b)
  a + b          # implicitly returned
end

fn abs(n)
  return -n when n < 0
  n              # implicitly returned
end

println add(3, 4)  # 7
println abs(-5)    # 5
```

`return` with no value (or `return null`) exits the function and returns `null`.

```kiwi
fn early_exit(x)
  return when x < 0   # returns null
  x * 2
end
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

### Variadic Parameters

Prefix the last parameter with `*` to make it variadic. It collects any extra positional arguments into a list.

```kiwi
fn sum(*args)
  total = 0
  for x in args do
    total += x
  end
  return total
end

println sum()           # 0
println sum(1, 2, 3)    # 6
println sum(10, 20, 30) # 60
```

Variadic parameters can follow regular parameters:

```kiwi
fn describe(label, *values)
  println "${label}: ${values}"
end

describe("nums", 1, 2, 3) # nums: [1, 2, 3]
describe("empty")          # empty: []
```

#### Splat operator

Use `*expr` at the call site to expand a list into individual arguments:

```kiwi
nums = [1, 2, 3]
println sum(*nums)  # 6

fn add(a, b, c)
  return a + b + c
end

println add(*nums)  # 6
```

### Hoisting

Top-level functions are **hoisted**. They are registered before any code in the script runs, so you can call a function that is defined further down in the file.

```kiwi
println greet("Kiwi")  # Hello, Kiwi!

fn greet(name)
  return "Hello, ${name}!"
end
```

Mutual recursion works for the same reason. In the example below, both functions are available before either is called:

```kiwi
println is_even(4)  # true
println is_odd(3)   # true

fn is_even(n)
  return true when n == 0
  return is_odd(n - 1)
end

fn is_odd(n)
  return false when n == 0
  return is_even(n - 1)
end
```

Hoisting applies only to **top-level** function definitions. Functions defined inside other functions, loops, or conditional blocks are not hoisted.

### Scope

Functions use lexical scoping — each call gets its own local scope, and outer (global) variables are readable and writable from inside. See [Scoping](scoping.md) for the complete rules.

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

### Nested Functions

A `fn` declaration inside another function is local to that function — it is callable anywhere in the enclosing body but does not leak into outer scope after the function returns.

```kiwi
fn make_adder(n)
  fn add(x)
    return x + n
  end
  return add(10)  # 10 + n
end

println make_adder(5)  # 15
add(1)                 # error — add is not defined here
```

This is useful for encapsulating helper logic without polluting the global namespace.

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
