# Loops

Kiwi supports the following kinds of loops. You can use the `break` keyword to exit a loop. You can use the `next` keyword to skip to the next iteration.

## Table of Contents
1. [`repeat`](#repeat)
2. [`while`](#while)
3. [`for`](#for)

## `repeat`

The `repeat`-loop is used to loop `n` number of times where `n` is a positive non-zero integer.

```ruby
i = 0

repeat 10 do
  i += 1
end

println i # prints: 10
```

## `while`

The `while`-loop is used to loop based on a condition.

##### Loop Based on a Condition

```ruby
i = 0
while i <= 10 do
  i += 1
  println(i)
end
```

##### Infinite Loops

To loop indefinitely, the loop condition expression must evaluate to `true`.

```ruby
import "@kiwi/time" # for `delay()`
while true do
  print("\rPress Ctrl+C to exit!")
  time::delay(500) # sleep for 500 seconds
end
```

##### Exit a Loop

Exiting a loop with the `break` keyword.

```ruby
i = 0
while true do
  i += 1

  if i == 2
    break
  end
end
```

##### Loop Continuation
Skipping iterations with the `next` keyword.
```ruby
i = 0
while true do
  i += 1

  if i % 2 == 0
    next
  end

  println(i)

  if i >= 10
    break
  end
end
```

## `for`

In Kiwi, `for`-loops are used to iterate collections.

##### Iterating a Hash

Loop on the `.keys()` builtin to iterate the keys of a hash.

```ruby
# Iterate the keys in the hash.
for key in myHash.keys() do
  println("${key}: ${myHash[key]}")
end

# Iterate the keys in the hash, with an index.
for key, index in myHash.keys() do
  println("Key ${index}: ${key}")
end
```

Loop on the `.values()` builtin to iterate the values of a hash.

```ruby
# Iterate the values in the hash.
for value in myHash.values() do
  println(value)
end

# Iterate the values in the hash, with an index.
for value, index in myHash.values() do
  println("Value ${index}: ${value}")
end
```

##### Iterating a List

Use the `for` keyword to iterate a list.

```ruby
fruits = ["kiwi", "mango", "lime"]

# Iterate the values in the list.
for fruit in fruits do
  println(fruit)
end

# Iterate the values in the list, with an index.
for fruit, index in fruits do
  println("Fruit ${index}: ${item}")
end
```

##### Iterating a range

Use the `for` keyword to iterate a range.

```ruby
for i in [1..10] do
  println("${i}")
end
```

To learn more about collections, see [Hashes](hashes.md), [Lists](lists.md), and [Ranges](ranges.md).