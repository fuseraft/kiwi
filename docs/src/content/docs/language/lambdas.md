---
title: Lambdas and Closures
description: Anonymous functions, the arrow and block forms, closures, and passing functions around.
---

A lambda is a function without a name. Lambdas are values: store them in variables, pass them to functions, and return them.

## Writing a lambda

The **arrow form** holds a single expression:

```kiwi
square = do (n) => n * n
add = do (a, b) => a + b
answer = do () => 42

println square(5)    # 25
println add(2, 3)    # 5
println answer()     # 42
```

The **block form** holds any number of statements and returns the value of the last one. Leave out the arrow and close with `end`:

```kiwi
classify = do (n)
  if n > 0
    "positive"
  elsif n < 0
    "negative"
  else
    "zero"
  end
end

println classify(-3)   # negative
```

`with (params) do ... end` is an alternative spelling of the block form:

```kiwi
multiply = with (a, b) do
  a * b
end

println multiply(6, 7)   # 42
```

Lambda parameters support everything function parameters do: defaults, type hints, and a variadic `*` parameter.

```kiwi
greet = do (name = "you") => "Hi, ${name}"
total = with (*nums) do nums.sum() end

println greet()           # Hi, you
println total(1, 2, 3)    # 6
println total(*[4, 5])    # 9
```

A lambda can be called right where it's written:

```kiwi
println (do (x, y) => x * y)(6, 7)   # 42
```

`typeof` reports both lambdas and named functions as `lambda`.

## Passing lambdas

Higher-order list methods take a lambda:

```kiwi
words = ["kiwi", "banana", "fig"]

println words.filter(do (w) => w.size() > 3)            # ["kiwi", "banana"]
println words.map(do (w) => w.uppercase())              # ["KIWI", "BANANA", "FIG"]
println words.sort(do (a, b) => a.size() < b.size())    # ["fig", "kiwi", "banana"]
```

When an arrow lambda is an argument, it may end with an optional `end`, which some people find easier to scan:

```kiwi
println [1 to 6].filter(do (n) => n % 2 == 0 end)   # [2, 4, 6]
```

Named functions can be passed the same way, with no conversion:

```kiwi
fn is_even(n)
  n % 2 == 0
end

println [1 to 6].filter(is_even)   # [2, 4, 6]
```

Your own functions can accept and call lambdas:

```kiwi
fn retry(times, action)
  result = null
  repeat times as attempt do
    result = action(attempt)
    break when result
  end
  result
end

println retry(5, do (n) => n >= 3)   # true
```

Every callable also has a `.call(arguments)` method that takes the arguments as a list:

```kiwi
double = do (x) => x * 2
println double.call([21])   # 42
```

## Closures

A lambda can use variables from the place where it was created, and it keeps them alive after that function returns. Changes go both ways: the lambda sees later updates, and its own updates are visible outside.

```kiwi
fn make_counter()
  count = 0
  do ()
    count += 1
    count
  end
end

counter = make_counter()
counter()
counter()
println counter()       # 3

other = make_counter()
println other()         # 1
```

Each call to `make_counter` creates a separate `count`, so the two counters are independent.

Lambdas that update top-level variables work the same way:

```kiwi
total = 0
[1, 2, 3].each(do (n) => total += n)
println total   # 6
```

### Closures in loops

Each iteration of a `for` loop gives its closures their own copy of the loop variable, so a lambda created in the loop keeps the value from its iteration:

```kiwi
callbacks = []
for i in [1 to 3] do
  callbacks.push(do () => i)
end
println callbacks.map(do (f) => f())   # [1, 2, 3]
```

Other variables assigned in the loop body are shared, as usual: a lambda sees their latest value.

## Returning a lambda

A function can return a lambda as its last expression, in either form:

```kiwi
fn make_adder(n)
  do (x) => x + n
end

add5 = make_adder(5)
println add5(10)   # 15
```

An arrow lambda may also be closed with its own `end` on the same line (`do (x) => x + n end`); an `end` on a later line closes the enclosing block.

The block form:

```kiwi
fn make_multiplier(n)
  do (x)
    x * n
  end
end

println make_multiplier(3)(4)   # 12
```
