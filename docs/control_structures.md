
# Control Structures

Control structures are used to control the flow of execution within Kiwi. 

## Table of Contents
1. [`if`](#if)
2. [`when`](#when)
    1. [`break`](loops.md#exit-a-loop)
    2. [`exit`](#exit)
    3. [`next`](loops.md#loop-continuation)
    4. [`return`](functions.md#return-value)
    5. [`throw`](error_handling.md#throwing-an-error)
3. [`while`](loops.md#while)
4. [`?: (ternary operator)`](#ternary-operator)

### `exit`

The `exit` keyword is used to exit the program.

The value passed to exit must be an integer and represents the exit code.

### `if`

An *if-statement* is used to define code blocks that should only be executed conditionally. 

It is crafted using the following keywords:

- `if`: keyword used to define the first conditional path. 
- `elsif`: keyword used to define alternate conditional paths.
- `else`: keyword used to define the default conditional path.
- `end`: keyword for ending blocks.

#### Syntax: `if`

```kiwi
if condition
    [ statements ]
[ elsif elsif_condition
    [ elsif_statements ] ]
[ else
    [ else_statements ] ]
end
```

#### Example: `if`
```kiwi
a = 0, b = 1

if a < b
  println("${a} is less than ${b}")
elsif a > b
  println("${a} is greater than ${b}")
else
  println("${a} is equal to ${b}")
end
```

### `when`

The `when` keyword is used to define a condition for the following control structures:

- [`break`](loops.md#exit-a-loop): Break out of a loop.
- [`exit`](#exit): Exit the program.
- [`next`](loops.md#loop-continuation): Begin the next loop iteration.
- [`return`](functions.md#return-value): Return from a function or method.
- [`throw`](error_handling.md#throwing-an-error): Throw an error.

#### Syntax: `when`

```kiwi
control_structure [ expression ] when condition
```

#### Example: `when`

```kiwi
def sum(list)
  throw "Cannot sum a non-List value." when !list.is_a(List)
  return 0 when list.empty()
  return list.sum()
end

println(sum([1, 2])) # prints: 3
println(sum([]))     # prints: 0
```

### `?:` (ternary operator)

#### Syntax: `?:`

```kiwi
condition ? true_expression : false_expression
```

#### Example: `?:`

```kiwi
def min(a, b)
  return a < b ? a : b
end

println(min(5, 2))  # prints: 2
println(min(1, 10)) # prints: 1
```