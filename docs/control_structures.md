
# Control Structures

Control structures are used to control the flow of execution within Kiwi. 

## Table of Contents
1. [`if`](#if)
2. [`case`](#case)
3. [`when`](#when)
    1. [`break`](loops.md#exit-a-loop)
    2. [`exit`](#exit)
    3. [`next`](loops.md#loop-continuation)
    4. [`return`](functions.md#return-value)
    5. [`throw`](error_handling.md#throwing-an-error)
4. [`while`](loops.md#while)
5. [`?: (ternary operator)`](#ternary-operator)

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

### `case`

The `case` keyword is used to define a case statement.

#### Example: `case`

```hayward
dt = time::now()

# A `case` statement without a test.
case
    when dt.hour() < 12
        println "good morning"
    when dt.hour() < 17
        println "good afternoon"
    when dt.hour() < 21
        println "good evening"
    else
        println "good night"
end

# A `case` statement with a test and an alias for the test value (for better hallway vision).
case dt.hour() as hour
    when hour < 12
        println "good morning"
    when hour < 17
        println "good afternoon"
    when hour < 21
        println "good evening"
    else
        println "good night, the time is: ${dt}"
end

# Using a `case` statement to assign a value conditionally.
x = case dt.hour() as hour
        when hour < 12 "morning"
        when hour < 17 "afternoon"
        when hour < 21 "evening"
        else "night"
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
fn sum(list)
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
fn min(a, b)
  return a < b ? a : b
end

println(min(5, 2))  # prints: 2
println(min(1, 10)) # prints: 1
```