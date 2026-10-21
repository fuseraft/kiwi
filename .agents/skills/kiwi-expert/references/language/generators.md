# Generators

> Functions that produce a lazy sequence of values with yield.

A generator is a function that produces a sequence of values one at a time. Values are computed only when a loop asks for the next one, so a generator can describe a huge or even infinite sequence without building it in memory.

## Writing a generator

Any function that contains `yield` is a generator function. Calling it doesn't run the body; it returns a `generator` object. A `for` loop then runs the body up to each `yield`, takes the yielded value, and resumes from there on the next iteration. The loop ends when the function returns.

```kiwi
fn countdown(n)
  while n > 0 do
    yield n
    n -= 1
  end
end

for x in countdown(3) do
  println x
end
# 3
# 2
# 1

println typeof(countdown(3))   # generator
```

`yield` with no value yields `null`. `return` ends the sequence early.

```kiwi
fn first_two()
  yield "a"
  yield "b"
  return
  yield "never"
end

for item, i in first_two() do
  println "${i}: ${item}"   # 0: a, 1: b
end
```

As in any `for` loop, a second variable receives the index.

## Infinite sequences

Because values are produced on demand, a generator can loop forever. The consumer decides when to stop:

```kiwi
fn fibonacci()
  a, b = 0, 1
  while true do
    yield a
    a, b = b, a + b
  end
end

for f in fibonacci() do
  break when f > 50
  print "${f} "
end
println ""   # 0 1 1 2 3 5 8 13 21 34
```

## Filtering and transforming

Generators compose like pipelines. Each stage pulls values from the previous one:

```kiwi
fn naturals()
  n = 1
  while true do
    yield n
    n += 1
  end
end

fn squares_of_odds(source)
  for n in source do
    next when n % 2 == 0
    yield n * n
  end
end

for s in squares_of_odds(naturals()) do
  break when s > 100
  print "${s} "
end
println ""   # 1 9 25 49 81
```

## Generator objects

A generator object can be stored and iterated later. Each call creates an independent generator with its own position. Once a generator has run to the end it stays exhausted; iterate a fresh one to start over.

```kiwi
fn letters()
  yield "x"
  yield "y"
end

gen = letters()
for l in gen do
  print l
end
for l in gen do
  print l   # nothing: gen is exhausted
end
println ""  # xy

for l in letters() do
  print l
end
println ""  # xy
```

To collect a generator's values into a list, push them in a loop:

```kiwi
fn range_step(start, stop, step)
  i = start
  while i < stop do
    yield i
    i += step
  end
end

values = []
for v in range_step(0, 20, 5) do
  values.push(v)
end
println values   # [0, 5, 10, 15]
```

Struct methods and lambdas can be generators too:

```kiwi
struct Countdown
  fn new(from)
    @from = from
  end

  fn ticks()
    n = @from
    while n > 0 do
      yield n
      n -= 1
    end
  end
end

for t in Countdown.new(3).ticks() do
  println t
end

squares = do (n)
  for i in [1 to n] do
    yield i * i
  end
end
for s in squares(3) do
  println s
end
```
