# Lambdas and Closures

> Anonymous functions, the arrow and block forms, closures, and passing functions around.

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

:::caution[Closures in loops]
Lambdas created in a `for` loop share the loop variable. They all see its final value:

```kiwi
callbacks = []
for i in [1 to 3] do
  callbacks.push(do () => i)
end
println callbacks.map(do (f) => f())   # [3, 3, 3]
```

To capture each value, create the lambda in a function that takes the value as a parameter.
:::

## Returning a lambda

To return an arrow lambda from a function or block, close the arrow lambda with its own `end`. Otherwise it takes the enclosing block's `end` as its own and the block is reported as unclosed.

```kiwi
fn make_adder(n)
  do (x) => x + n end
end

add5 = make_adder(5)
println add5(10)   # 15
```

The block form doesn't have this problem:

```kiwi
fn make_multiplier(n)
  do (x)
    x * n
  end
end

println make_multiplier(3)(4)   # 12
```
