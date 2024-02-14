# Loops

There are a handful of ways to loop so far.

1. `while`
2. `for`

### `while`-loops

Loop based on a condition. *See the test script for a better example with nesting.*

```ruby
@i = 0
while @i <= 10 do
  @i += 1
  println @i
end
```

Loop indefinitely.
```ruby
while true do
    println "Press Ctrl+C to exit!"
end
```

### `for`-loops

Loop on a range.
```ruby
for @i in [1..10] do
  println "${@i}"
end
```

Loop through a collection.

```ruby
@fruits = ["kiwi", "mango", "lime"]
for @fruit, @index in @list do
  println "Fruit ${@index}: ${@item}"
end
```

To learn more about collections, see [lists](lists.md).