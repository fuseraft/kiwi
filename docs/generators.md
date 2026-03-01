# Generators

A **generator** is a function that can pause its execution and yield a sequence of values one at a time, resuming where it left off each time the next value is requested.

Generators are useful for:

- Producing sequences lazily (values computed on demand, not all at once)
- Infinite sequences (e.g. Fibonacci numbers, event streams)
- Memory-efficient pipelines over large data sets

## Defining a generator

Any function that contains at least one `yield` statement is a **generator function**. Calling it returns a generator object rather than executing the body immediately.

```kiwi
fn counter(n)
  var i = 0
  while i < n do
    yield i
    i += 1
  end
end
```

## `yield`

The `yield` keyword suspends the function and produces the given value to the caller. The function body resumes from that point the next time a value is requested.

```kiwi
fn greet()
  yield "Hello"
  yield "World"
end
```

Yielding nothing produces a `null` value:

```kiwi
fn silent()
  yield        # yields null
  yield "ok"
end
```

## Iterating a generator with `for`

Pass a generator call directly to a `for` loop. The loop advances the generator one step per iteration and stops automatically when the generator is exhausted.

```kiwi
for word in greet() do
  println word
end
# Hello
# World
```

### With an index variable

```kiwi
for word, i in greet() do
  println "${i}: ${word}"
end
# 0: Hello
# 1: World
```

## Infinite generators

Because values are produced lazily, a generator can yield infinitely. Use `break` (optionally with `when`) to stop consuming it.

```kiwi
fn fibonacci()
  var a = 0
  var b = 1
  while true do
    yield a
    var tmp = a + b
    a = b
    b = tmp
  end
end

for f in fibonacci() do
  println f
  break when f > 100
end
# 0 1 1 2 3 5 8 13 21 34 55 89 144
```

## Parameters

Generator functions accept parameters just like regular functions.

```kiwi
fn range_step(start, stop, step)
  var i = start
  while i < stop do
    yield i
    i += step
  end
end

for x in range_step(0, 20, 3) do
  print "${x} "
end
# 0 3 6 9 12 15 18
```

## Storing a generator

You can store a generator in a variable and iterate it later. Each variable holds an independent generator instance.

```kiwi
var evens = range_step(0, 10, 2)
var odds  = range_step(1, 10, 2)

for x in evens do
  print "${x} "
end
# 0 2 4 6 8

for x in odds do
  print "${x} "
end
# 1 3 5 7 9
```

## `typeof`

`typeof` on a generator object returns `"generator"`.

```kiwi
var g = counter(5)
println typeof(g)  # generator
```

## How generators work

When a generator function is called it does **not** execute any of its body. Instead, it returns a generator object. Each iteration of a `for` loop (or each call requesting the next value) resumes the body from after the last `yield` until it hits the next `yield` or the function returns. Once the body returns, the generator is exhausted and the `for` loop ends.

This is analogous to Python generators and C# `IEnumerable`/`yield return`.

## Examples

### Sieve of Eratosthenes (lazy prime sequence)

```kiwi
fn naturals(start)
  var n = start
  while true do
    yield n
    n += 1
  end
end

fn primes()
  var candidates = naturals(2)
  for p in candidates do
    yield p
    # filter out multiples — illustrative; a full sieve needs pipelines
    break
  end
end
```

### Chunked file processing

```kiwi
fn read_chunks(filename, size)
  var f = fio::read_bytes(filename)
  var i = 0
  while i < f.size() do
    yield f.slice(i, i + size)
    i += size
  end
end

for chunk in read_chunks("data.bin", 1024) do
  process(chunk)
end
```

## See also

- [Loops](loops.md)
- [Functions](functions.md)
- [Lambdas](lambdas.md)
- [Control Structures](control_structures.md)
