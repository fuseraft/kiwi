# Variables

Variables are by default dynamically typed, meaning they do not require explicit type declarations. The type of a variable is inferred from the value assigned to it. For more information on types, please see [Types](types.md).

# `var` Keyword

The `var` keyword in Kiwi simplifies variable declaration, supporting multiple styles of initialization with optional type hints. It offers a clear, compact syntax for declaring one or more variables at once.

## Syntax

```kiwi
var (<declaration_1>, <declaration_2>, ..., <declaration_n>)
```

### Declaration Options
Each declaration can take one of the following forms:
1. **Regular declaration with initialization:**
   ```kiwi
   var_name = value
   ```
   - Infers the variable type from the assigned value.

2. **Type-hinted declaration with initialization:**
   ```kiwi
   var_name: type = value
   ```
   - Specifies the variable type explicitly with an initializer.

3. **Type-hinted declaration without initialization:**
   ```kiwi
   var_name: type
   ```
   - Defaults to the type’s default value (e.g., `0` for integers, `false` for booleans, `null` for others).

4. **Uninitialized declaration:**
   ```kiwi
   var_name
   ```
   - Defaults to `null`.

### Multiple Declarations
You can group multiple declarations within parentheses `()` separated by commas `,`:
```kiwi
var (x = 10, y: float = 3.14, z: string, w)
```

---

## Examples

### Single Declaration
```kiwi
var (x = 42)            # `x` is an integer with the value 42
var (name: string)       # `name` is a string initialized to null
var (flag: boolean = true) # `flag` is a boolean with the value true
```

### Multiple Declarations
```kiwi
var (
  age: integer = 25,    # `age` is an integer with value 25
  name = "Kiwi",        # `name` is a string with value "Kiwi"
  is_active: boolean,   # `is_active` is a boolean, defaults to false
  count                # `count` is uninitialized, defaults to null
)
```

### Practical Example
```kiwi
fn initialize_variables()
  var (
    counter: integer = 0,
    name: string = "Test",
    active: boolean = true,
    data: list = [1, 2, 3],
    metadata: hashmap = {"key": "value"}
  )

  return [counter, name, active, data, metadata]
end
```

---

## Key Features
1. **Type Hints:** Declare variables with explicit types for clarity and error prevention.
2. **Initialization Flexibility:** Mix initialized and uninitialized variables within a single declaration.
3. **Compact Syntax:** Group related variable declarations for cleaner, more maintainable code.

---

## Use Cases
- **Convenient Debugging:** Quickly declare multiple variables for experimentation or testing.
- **Readable Code:** Clearly communicate intent through grouped declarations and type hints.
- **Efficient Initialization:** Avoid repetitive `null` initializations or redundant type declarations.

## Assigning Variables

You can assign a variable in Kiwi using the `=` operator. 

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

### Variable Reassignment
You can reassign variables at any point, and the type can change based on the new value:

```kiwi
x = 5         # `x` is initially an integer
x = "Kiwi"    # `x` is now a string
```

### Multiple Assignments
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

## Variable Scope
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
