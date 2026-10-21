# Functions

> Defining functions, parameters, defaults, named and variadic arguments, return values and recursion.

## Defining a function

`fn` defines a function. (`def` is an alias.) The body runs until `end`.

```kiwi
fn greet(name)
  println "Hello, ${name}!"
end

greet("Kiwi")   # Hello, Kiwi!
```

## Return values

A function returns the value of the last expression it evaluates, so most functions don't need `return`. Use `return` to leave early. A bare `return` returns `null`.

```kiwi
fn add(a, b)
  a + b
end

fn absolute(n)
  return -n when n < 0
  n
end

println add(3, 4)      # 7
println absolute(-5)   # 5
```

To return several values, return a list and unpack it:

```kiwi
fn min_max(values)
  [values.min(), values.max()]
end

low, high = min_max([4, 1, 9])
println "${low}..${high}"   # 1..9
```

## Default values

A parameter can have a default, used when the argument is left out. Defaults are evaluated on each call, so a default list or hashmap is fresh every time, and a default can refer to earlier parameters.

```kiwi
fn greet(name = "stranger", greeting = "Hello")
  "${greeting}, ${name}!"
end

println greet()               # Hello, stranger!
println greet("Ada")          # Hello, Ada!

fn box(width, height = width)
  "${width}x${height}"
end

println box(3)                # 3x3
```

## Named arguments

Any argument can be passed by name, as `name: value` or `name = value`. Positional arguments fill the remaining parameters from left to right, so the two styles mix freely.

```kiwi
fn connect(host, port = 80, secure = false)
  "${secure ? "https" : "http"}://${host}:${port}"
end

println connect("example.com")                         # http://example.com:80
println connect("example.com", secure: true, port: 443) # https://example.com:443
println connect(port = 8080, host = "localhost")       # http://localhost:8080
```

Passing a name that isn't a parameter is an error.

Kiwi checks the arguments a call passes. Leaving out a parameter that has no default, passing more positional arguments than there are parameters, or passing a parameter both by position and by name is an error:

```kiwi
fn connect(host, port = 80)
  "${host}:${port}"
end

connect("example.com", 80, true)   # ParameterCountMismatchError: Expected 1 to 2 but received 3.
```

[Lambdas](lambdas.md) are more forgiving, so they work as callbacks: a missing argument is `null` and extra arguments are ignored.

## Variadic parameters

Prefix the last parameter with `*` to collect any remaining positional arguments into a list:

```kiwi
fn sum(*numbers)
  total = 0
  for n in numbers do
    total += n
  end
  total
end

println sum()            # 0
println sum(1, 2, 3)     # 6

fn tag(label, *values)
  "${label}: ${values}"
end

println tag("nums", 1, 2)   # nums: [1, 2]
```

At the call site, `*list` spreads a list into separate arguments:

```kiwi
fn add3(a, b, c)
  a + b + c
end

args = [1, 2, 3]
println add3(*args)   # 6
```

## Hoisting

Top-level functions are registered before the script starts running, so a function can be called above the line that defines it. Mutual recursion works the same way.

```kiwi
println is_even(10)   # true

fn is_even(n)
  return true when n == 0
  is_odd(n - 1)
end

fn is_odd(n)
  return false when n == 0
  is_even(n - 1)
end
```

Functions defined inside other functions or blocks are created when that code runs.

## Recursion

```kiwi
fn factorial(n)
  return 1 when n <= 1
  n * factorial(n - 1)
end

println factorial(10)   # 3628800
```

Recursion is limited to roughly 500 nested calls; deeper recursion raises a `Stack overflow` error. Rewrite very deep recursion as a loop.

## Functions as values

A named function is a value. Pass it to another function, store it in a variable or collection, or return it. Call it like any [lambda](lambdas.md).

```kiwi
fn shout(text)
  text.uppercase() + "!"
end

fn apply_twice(f, value)
  f(f(value))
end

println apply_twice(shout, "hi")       # HI!!
println ["a", "b"].map(shout)          # ["A!", "B!"]

handlers = {loud: shout}
println handlers["loud"]("hey")        # HEY!
```

## Nested functions

A `fn` inside another function is a helper for that function. It exists only while the outer call runs:

```kiwi
fn summarize(values)
  fn average(xs)
    xs.sum() / xs.size()
  end

  "n=${values.size()} mean=${average(values)}"
end

println summarize([2, 4, 6])   # n=3 mean=4
```

## Related pages

- [Lambdas](lambdas.md) for anonymous functions and closures.
- [Type Hints](type-hints.md) for annotating parameters and return values.
- [Generators](generators.md) for functions that `yield` a sequence.
- [Decorators](decorators.md) for wrapping functions with `@name`.
- [Scoping](scoping.md) for how functions see variables.
