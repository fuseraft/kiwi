# Loops

Kiwi supports the following kinds of loops. You can use the `break` keyword to exit a loop. You can use the `next` keyword to skip to the next iteration.

## Table of Contents
1. [`while`](#while)
2. [`for`](#for)

## `while`

The `while`-loop is used to loop based on a condition.

*See the test script for a better example with nesting.*

##### Loop Based on a Condition

```ruby
i = 0
while i <= 10 do
  i += 1
  println i
end
```

##### Infinite Loops

To loop indefinitely, the loop condition expression must evaluate to `true`.

```ruby
while true do
  println "Press Ctrl+C to exit!"
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

  println i

  if i >= 10
    break
  end
end
```

## `for`

In Kiwi, `for`-loops are used to iterate collections.

##### Iterating a Hash

Use the `for` keyword and the `.keys()` Hash-builtin to iterate a hash.

```ruby
# Iterate the values in the list.
for key in myHash.keys() do
  println myHash[key]
end

# Iterate the values in the list, with an index.
for key, index in myHash.keys() do
  println "Key ${index}: ${key}"
end
```

##### Iterating a List

Use the `for` keyword to iterate a list.

```ruby
fruits = ["kiwi", "mango", "lime"]

# Iterate the values in the list.
for fruit in fruits do
  println fruit
end

# Iterate the values in the list, with an index.
for fruit, index in fruits do
  println "Fruit ${index}: ${item}"
end
```

##### Iterating a range

Use the `for` keyword to iterate a range.

```ruby
for i in [1..10] do
  println "${i}"
end
```

To learn more about collections, see [Hashes](hashes.md), [Lists](lists.md), and [Ranges](ranges.md).