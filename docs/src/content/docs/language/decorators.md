---
title: Decorators
description: Wrapping functions with @decorator syntax.
---

A decorator wraps a function with extra behavior: logging, timing, caching, retries, registration. Write `@name` on the line above a `fn`.

```kiwi
fn shout(f)
  with (*args) do
    f(*args).uppercase()
  end
end

@shout
fn greet(name)
  "hello, ${name}"
end

println greet("world")   # HELLO, WORLD
```

A decorator is an ordinary function. It receives the decorated function and returns the function that should replace it. `@shout` above `fn greet` means the same as writing `greet = shout(greet)` after the definition.

Most decorators return a variadic lambda, `with (*args) do ... end`, that calls the original with `f(*args)`, so they work with functions of any arity.

## Stacking

Several decorators can be stacked. The one closest to `fn` is applied first:

```kiwi
fn double_result(f)
  with (*args) do f(*args) * 2 end
end

fn add_one(f)
  with (*args) do f(*args) + 1 end
end

@add_one
@double_result
fn multiply(a, b)
  a * b
end

println multiply(3, 4)   # 25, that is (3 * 4) * 2 + 1
```

## Decorator arguments

Arguments after the decorator name are passed to the decorator after the function. Positional and named arguments both work.

```kiwi
fn repeat_call(f, times)
  with (*args) do
    result = null
    repeat times do
      result = f(*args)
    end
    result
  end
end

fn label(f, prefix = "LOG")
  with (*args) do
    "[${prefix}] ${f(*args)}"
  end
end

clicks = 0

@repeat_call(3)
fn click()
  clicks += 1
end

@label(prefix: "INFO")
fn status(code)
  "status ${code}"
end

click()
println clicks        # 3
println status(200)   # [INFO] status 200
```

## Decorators from packages

Decorators defined in a package are referenced with `::`. The standard library provides several:

```kiwi
@tester::test("addition works")
fn test_addition()
  tester::assert_eq(1 + 1, 2, "basic addition")
end

tester::run_tests()
```

| Decorator | Purpose |
| :-- | :-- |
| `@tester::test(name)` | Registers a [unit test](/kiwi/tools/testing/) |
| `@bench::mark(name)` | Registers a [benchmark](/kiwi/stdlib/bench/) |
| `@bench::timed` | Prints the elapsed time of every call |
| `@bench::profile(name, iterations, warmup)` | Benchmarks the function once, when it is defined |

## Examples

### Timing

```kiwi
fn timed(f)
  with (*args) do
    started = time::ticks()
    result = f(*args)
    elapsed = time::ticksms(time::ticks() - started)
    println "took ${elapsed.to_string("f2")} ms"
    result
  end
end

@timed
fn slow_sum(n)
  [1 to n].sum()
end

println slow_sum(100000)
```

### Memoizing

```kiwi
fn memoize(f)
  cache = {}
  with (n) do
    cache[n] = f(n) when !cache.has_key(n)
    cache[n]
  end
end

@memoize
fn fib(n)
  return n when n < 2
  fib(n - 1) + fib(n - 2)
end

println fib(60)   # 1548008755920
```

Because the decorated function replaces the original name, the recursive calls inside `fib` also go through the cache.

## Decorating methods

Decorators work on struct methods too. The decorated method still runs with `@` as the instance it was called on. Modifiers such as `static` and `private` go between the decorators and `fn`. A bare `@name` line whose name the struct assigns as an instance variable (`@name = ...`) is not a decorator.

```kiwi
fn logged(f)
  do (*args)
    println "calling with ${args}"
    f(*args)
  end
end

struct Account
  fn new(balance)
    @balance = balance
  end

  @logged
  fn deposit(amount)
    @balance += amount
  end
end

acct = Account.new(10)
println acct.deposit(5)
```

```
calling with [5]
15
```
