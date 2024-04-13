# Lambdas

Lambdas are used to treat functions as first-class citizens. You can pass lambdas as method parameters.

You can define a lambda using the `with` keyword.

For the examples below, I am using a list of hashes called `list`.

```ruby
# Define a list of hashes to work with in our example.
list = [
  {"id": 0}, {"id": 1}, {"id": 2}, {"id": 3}, {"id": 4},
  {"id": 5}, {"id": 6}, {"id": 7}, {"id": 8}, {"id": 9}
]
```

### Inline Lambdas

Lambdas can be used inline (without assignment).

```ruby
println list.select(with (item) do return item["id"] % 2 == 0 end)
# prints: [{"id": 0}, {"id": 2}, {"id": 4}, {"id": 6}, {"id": 8}]
```

### Assigned Lambdas

You can assign a reference to a lambda for reuse.

```ruby
odd_item_id = with (item) do
  return item["id"] % 2 != 0
end

println list.select(odd_item_id)
# prints: [{"id": 1}, {"id": 3}, {"id": 5}, {"id": 7}, {"id": 9}]
```

### Passing Lambdas as Parameters

You can pass lambdas as parameters to methods.

```ruby
puts = with (s) do
  println s
end

puts("Hello, World!") # prints: Hello, World!

def use_lambda(print_func, message)
  print_func(message)
end

use_lambda(puts, "Hello, Astral!") # prints: Hello, Astral!

println("")
```
