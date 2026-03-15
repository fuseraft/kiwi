# Decorators

Decorators let you wrap a function with additional behaviour using `@name` syntax placed above a `fn` definition. They are a clean alternative to manually writing `f = wrapper(f)`.

---

## Basic decorator

A decorator is any callable that accepts a function as its first argument and returns a new function. The `@name` syntax calls it automatically.

```kiwi
fn shout(f)
  return with (*args) do
    result = f(*args)
    return result.to_string().uppercase()
  end
end

@shout
fn greet(name)
  return "hello, ${name}"
end

println greet("world")  # HELLO, WORLD
```

> The decorator receives the original function as a lambda and must return the replacement callable.

---

## How it works

`@decorator` before `fn f` is equivalent to:

```kiwi
fn f(...)
  ...
end
f = decorator(f)
```

---

## Stacking decorators

Multiple decorators can be stacked — they are applied **bottom-to-top** (the one closest to `fn` is applied first).

```kiwi
fn double_result(f)
  return with (*args) do f(*args) * 2 end
end

fn add_one(f)
  return with (*args) do f(*args) + 1 end
end

@add_one
@double_result
fn mul(a, b)
  return a * b
end

# mul(3, 4) = 12  →  double_result: 24  →  add_one: 25
println mul(3, 4)  # 25
```

---

## Decorators with arguments

Pass extra arguments after the decorator name. They are forwarded after the wrapped function.

```kiwi
fn ntimes(f, n)
  return with (*args) do
    result = null
    for i in [1..n] do
      result = f(*args)
    end
    return result
  end
end

count = 0

@ntimes(3)
fn tick()
  count += 1
end

tick()
println count  # 3
```

Arguments can also be passed by name using `name: value` or `name = value` syntax.

```kiwi
fn tag(f, value: string): lambda
  return with (*args) do
    println "${value}: ${f(*args)}"
  end
end

@tag(value: "INFO")
fn compute(x)
  return x * 2
end

compute(5)  # INFO: 10
```

Positional and keyword arguments can be mixed — positional args are filled left-to-right into unfilled parameter slots.

```kiwi
fn ntimes_labeled(f, n, label: string): lambda
  return with (*args) do
    for i in [1..n] do f(*args) end
    println "ran ${label} ${n} times"
  end
end

@ntimes_labeled(3, label: "benchmark")
fn work()
  # ...
end

work()  # ran benchmark 3 times
```

---

## Timing decorator

```kiwi
fn timer(f)
  return with (*args) do
    t1 = time::ticks()
    result = f(*args)
    elapsed = time::ticks() - t1
    println "Elapsed: ${elapsed}ms"
    return result
  end
end

@timer
fn slow_add(a, b)
  return a + b
end

println slow_add(10, 32)
# Elapsed: ...ms
# 42
```

---

## Logging decorator

```kiwi
fn logger(f)
  return with (*args) do
    println "Calling with ${args.size()} arg(s)"
    return f(*args)
  end
end

@logger
@timer
fn multiply(a, b)
  return a * b
end

println multiply(6, 7)
# Calling with 2 arg(s)
# Elapsed: ...ms
# 42
```

---

## Decorator on a struct method

Decorators work inside `struct` bodies too.

```kiwi
struct Counter
  fn new()
    @value = 0
  end

  fn logged(f)
    return with (*args) do
      println "calling method"
      return f(*args)
    end
  end

  @logged
  fn increment()
    @value += 1
  end
end

c = Counter.new()
c.increment()  # calling method
```

---

## Package-qualified decorators

Decorators from packages are referenced with `::` notation.

```kiwi
import "bench"
import "math"

@bench::mark("sort 500 items")
fn sort_bench()
  math::random_set(0, 5000, 500).sort()
end

@bench::timed
fn slow_sum(n)
  total = 0
  for i in [1..n] do total += i end
  return total
end

bench::run_all()
slow_sum(50000)
```

See [`bench` library](lib/bench.md) for `@bench::mark`, `@bench::profile`, and `@bench::timed`.

---

---

## Unit test registration

`@tester::test` is a built-in decorator that registers a function as a named unit test and returns it unchanged. It is the idiomatic way to write test suites in Kiwi.

```kiwi
import "tester"

@tester::test("addition works")
fn test_addition()
  tester::assert_eq(1 + 1, 2, "basic addition")
end

@tester::test("string concat")
fn test_strings()
  tester::assert("hello" + " world" == "hello world", "concat")
end

results = tester::run_tests()
```

The functions remain directly callable (`test_addition()`, `test_strings()`) in addition to being registered in the test runner.

See [`tester` library](lib/tester.md) for `assert`, `assert_eq`, and `run_tests`.

---

## See also

- [Functions](functions.md) — variadic parameters, splat
- [Lambdas](lambdas.md) — `with`/`do` block syntax
