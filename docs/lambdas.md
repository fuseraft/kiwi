# Lambdas

Lambdas are used to treat functions as first-struct citizens. You can pass lambdas as method parameters.

You can define a lambda using the `with` keyword.

For demonstration, we will define a list of hashmaps called `id_list` to be used in the examples below.

```kiwi
# Define a list of hashmaps to work with in our example.
id_list = [
  {"id": 0}, {"id": 1}, {"id": 2}, {"id": 3}, {"id": 4},
  {"id": 5}, {"id": 6}, {"id": 7}, {"id": 8}, {"id": 9}
]
```

### Inline Lambdas

Lambdas can be used inline (without assignment).

```kiwi
println(id_list.filter(with (item) do return item["id"] % 2 == 0 end))
# prints: [{"id": 0}, {"id": 2}, {"id": 4}, {"id": 6}, {"id": 8}]
```

### Assigned Lambdas

You can assign a reference to a lambda for reuse.

```kiwi
odd_item_id = with (item) do
  return item["id"] % 2 != 0
end

println(id_list.filter(odd_item_id))
# prints: [{"id": 1}, {"id": 3}, {"id": 5}, {"id": 7}, {"id": 9}]
```

### Passing Lambdas as Parameters

You can pass lambdas as parameters to methods.

```kiwi
puts = with (s) do
  println(s)
end

puts("Hello, World!") # prints: Hello, World!

fn use_lambda(print_func, message)
  print_func(message)
end

use_lambda(puts, "Hello, Kiwi!") # prints: Hello, Kiwi!
```
