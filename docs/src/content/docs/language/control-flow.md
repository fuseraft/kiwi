---
title: Control Flow
description: if, case, when guards, the ternary operator and exit.
---

## `if`, `elsif`, `else`

```kiwi
temperature = 18

if temperature > 25
  println "hot"
elsif temperature > 15
  println "mild"
else
  println "cold"
end
```

The condition can be any value. `false`, `null`, `0`, `0.0`, `""`, `[]` and `{}` are falsy and everything else is truthy; see [Truthiness](/kiwi/language/types/#truthiness).

`if` is a statement, not an expression. To choose between values, use a [`case`](#case) expression or the [ternary operator](#ternary).

## `case`

`case` is an expression: it evaluates to the value of the branch that matches. It comes in two forms.

**With a subject**, each `when` is compared to the value after `case`, like a `switch` in other languages. A `when` can list several values separated by commas.

```kiwi
fn describe(day)
  case day
    when "Saturday", "Sunday" "weekend"
    when "Friday"             "almost weekend"
    else                      "weekday"
  end
end

println describe("Sunday")    # weekend
println describe("Tuesday")   # weekday
```

**Without a subject**, each `when` is a condition, and the first true one wins:

```kiwi
hour = 14

greeting = case
  when hour < 12 "Good morning"
  when hour < 18 "Good afternoon"
  else           "Good evening"
end

println greeting   # Good afternoon
```

If nothing matches and there is no `else`, the result is `null`.

A colon after the `when` value is optional, and branches can hold several statements. As an expression, a branch evaluates to its last value.

```kiwi
code = 404

case code
  when 200
    println "OK"
  when 404
    println "Not Found"
    println "Check the URL."
  else
    println "Status ${code}"
end
```

### Ranges

`when low..high` (or `when low to high`) matches a value within an inclusive range. No list is built, and the bounds can be floats or negative.

```kiwi
fn grade(score)
  case score
    when 90..100: "A"
    when 80..89:  "B"
    when 70..79:  "C"
    else:         "F"
  end
end

println grade(95)   # A
println grade(72)   # C
println grade(10)   # F

temperature = -5.5
println case temperature
  when -273.15..0.0: "freezing"
  else:              "above freezing"
end                 # freezing
```

## `when` guards

Adding `when condition` to the end of a statement runs it only when the condition is truthy. Guards keep early exits on one line instead of wrapping them in `if` blocks.

```kiwi
fn safe_divide(a, b)
  throw "Division by zero" when b == 0
  a / b
end

for n in [1 to 10] do
  next when n % 2 == 0
  break when n > 7
  print "${n} "
end
println ""   # 1 3 5 7
```

Guards work on `return`, `throw`, `break`, `next`, `exit`, assignments and function or method calls:

```kiwi
fn log(message)
  println "LOG: ${message}"
end

verbose = true
log("starting") when verbose

count = 0
count += 1 when verbose
println count   # 1
```

The right-hand side of a guarded assignment is only evaluated when the condition is true, so it is safe to guard an expensive or failing call. If the guard is false the assignment doesn't happen at all, so a variable that didn't exist before still doesn't.

`return when condition` with no value returns `null`.

## Ternary

`condition ? when_true : when_false` is the inline choice between two values:

```kiwi
stock = 3
println stock > 0 ? "in stock" : "sold out"   # in stock
```

Ternaries nest, but a `case` usually reads better once there are more than two outcomes.

## `exit`

`exit code` ends the program immediately with that exit code. A bare `exit` exits with status `0`.

```kiwi nocheck
config_ok = false

exit 1 when !config_ok
println "never printed"
```

Uncaught errors also end the program, with exit code `1`. See [Error Handling](/kiwi/language/errors/).

## Loops

`for`, `while` and `repeat`, along with `break` and `next`, are covered in [Loops](/kiwi/language/loops/).
