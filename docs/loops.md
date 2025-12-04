# Loops

Kiwi supports the following kinds of loops. You can use the `break` keyword to exit a loop. You can use the `next` keyword to skip to the next iteration.

## Table of Contents
1. [`repeat`](#repeat)
2. [`while`](#while)
3. [`for`](#for)

## `repeat`

The `repeat`-loop is used to loop `n` number of times where `n` is a positive non-zero integer.

```kiwi
/#
# add 1 to `i` ten times, then print the value of `i`.
#/

i = 0

repeat 10 do
  i += 1
end

print i

# output: 10
```

Sometimes you need to know how many times a loop has executed. 

You can use the `as` keyword to specify an *iterator variable* which stores the number of times a loop has executed.

```kiwi
/#
# repeat 10 times, using `i` as the iterator variable.
# print the value of `i` followed by a space for each iteration.
#/

sum = 0

repeat 10 as i do
  sum += i
end

print sum

# output: 55
```

## `while`

The `while`-loop is used to loop based on a condition.

##### Loop Based on a Condition

```kiwi
i = 0
while i <= 10 do
  i += 1
  println(i)
end
```

##### Infinite Loops

To loop indefinitely, the loop condition expression must evaluate to `true`.

```kiwi
import "time" # for `delay()`
while true do
  print("\rPress Ctrl+C to exit!")
  time::delay(500) # sleep for 500 seconds
end
```

##### Exit a Loop

Exiting a loop with the `break` keyword.

```kiwi
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
```kiwi
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

##### Iterating a Hashmap

Loop on the `.keys()` builtin to iterate the keys of a hashmap.

```kiwi
# Iterate the keys in the hashmap.
for key in myHashmap.keys() do
  println("${key}: ${myHashmap[key]}")
end

# Iterate the keys in the hashmap, with an index.
for key, index in myHashmap.keys() do
  println("Key ${index}: ${key}")
end
```

Loop on the `.values()` builtin to iterate the values of a hashmap.

```kiwi
# Iterate the values in the hashmap.
for value in myHashmap.values() do
  println(value)
end

# Iterate the values in the hashmap, with an index.
for value, index in myHashmap.values() do
  println("Value ${index}: ${value}")
end
```

##### Iterating a List

Use the `for` keyword to iterate a list.

```kiwi
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

```kiwi
for i in [1 to 10] do
  println("${i}")
end
```

To learn more about collections, see [Hashmaps](hashmaps.md), [Lists](lists.md), and [Ranges](ranges.md).