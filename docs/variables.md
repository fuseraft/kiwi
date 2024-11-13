# Variables

Variables are dynamically typed, meaning they do not require explicit type declarations. The type of a variable is inferred from the value assigned to it. For more information on types, please see [Types](types.md).

##### Declaring and Assigning Variables
You can declare and assign a variable in Kiwi using the `=` operator. 

For example:

```kiwi
a = 10        # Integer assignment
b = 20.5      # Float assignment
c = "Hello"   # String assignment
d = [1, 2, 3] # List assignment
```

In these cases:
- `a` holds an integer value.
- `b` holds a floating-point number.
- `c` holds a string.
- `d` holds a list of integers.

##### Variable Reassignment
You can reassign variables at any point, and the type can change based on the new value:

```kiwi
x = 5         # `x` is initially an integer
x = "Kiwi"    # `x` is now a string
```

##### Multiple Assignments
Kiwi supports multiple variable assignments using the unpacking syntax `=<`. This allows assigning multiple values to multiple variables in a single line:

```kiwi
a, b, c =< true, {"a": false}, [1, 2, 3]
```

Here:
- `a` is assigned the boolean value `true`.
- `b` is assigned a hashmap with a single key-value pair.
- `c` is assigned a list containing three integers.

This can also be used with functions that return multiple values:

```kiwi
fn get_zero_and_one()
  return [0, 1]
end

zero, one =< get_zero_and_one()
```

Now, `zero` is `0` and `one` is `1`.

#### Variable Scope
Variables in Kiwi are function-scoped, meaning they are only accessible within the function where they are defined unless passed as arguments or returned from a function.

For example:

```kiwi
fn example_scope()
  x = 100  # `x` is only available within this function
end

println(x)  # null
```

#### String Interpolation with Variables
Kiwi supports string interpolation, allowing you to embed variable values directly into strings using `${}` syntax:

```kiwi
name = "Kiwi"
greeting = "Hello, ${name}!"
println(greeting)  # Hello, Kiwi!
```

This interpolation works with any valid expression inside `${}`.
