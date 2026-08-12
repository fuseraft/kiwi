# Control Flow in Kiwi

Kiwi offers a compact but very expressive set of control structures.

## Truthiness

A condition is any boolean expression. Non-boolean values are also valid — they are coerced to a boolean based on whether they are *truthy* or *falsy*:

| Type      | Truthy example  | Falsy example | Rule                       |
|-----------|-----------------|---------------|----------------------------|
| `integer` | `1`             | `0`           | Non-zero is truthy         |
| `float`   | `1.0`           | `0.0`         | Non-zero is truthy         |
| `boolean` | `true`          | `false`       | The boolean values         |
| `string`  | `"hello"`       | `""`          | Non-empty string is truthy |
| `list`    | `[1, 2, 3]`     | `[]`          | Non-empty list is truthy   |
| `hashmap` | `{"a": 1}`      | `{}`          | Non-empty hashmap is truthy|

## Quick Reference

| Construct              | Best for                              | Returns value? | Early-exit friendly? |
|------------------------|---------------------------------------|----------------|----------------------|
| `if / elsif / else`    | Classic branching                     | No             | Somewhat             |
| `case … when … else`   | Matching one value or many conditions | Yes            | Yes                  |
| `when` guard clauses   | Early returns, skips, errors          | —              | Yes                  |
| `?:` ternary           | Inline single-condition choice        | Yes            | No                   |
| `break / next when …`  | Loop control                          | —              | Yes                  |

## 1. `if` – `elsif` – `else`

Traditional conditional blocks.

```kiwi
if user.age < 18
  println "Access denied"
elsif user.banned
  println "Account suspended"
else
  grant_access(user)
end
```

## 2. `case` – `when` – `else`

Two common styles:

### A. Condition-only `case` (when conditions are unrelated)

```kiwi
hour = time::now().hour()
time_of_day = case
  when hour.between(5, 11) 'morning'
  when hour.between(12, 16) 'afternoon'
  when hour.between(17, 21) 'evening'
  else 'night'
end
```

### B. Value-tested `case` (similar to a `switch` statement in other languages)

```kiwi
greeting = case time_of_day
  when 'morning'    'Good morning!'
  when 'afternoon'  'Good afternoon.'
  when 'evening'    'Good evening.'
  when 'night'      'Hello, night owl!'
  else              'What time is it again?'
end
```

### C. Multiple values per `when` (comma-separated)

A `when` clause can match against multiple values by separating them with commas. The branch fires if the test value equals **any** of them.

```kiwi
mood = case day
  when "Monday"               "Here we go again."
  when "Friday"               "Almost there!"
  when "Saturday", "Sunday"   "Enjoy the weekend!"
  else                        "Just another day."
end
```

This also works in condition-only `case`:

```kiwi
label = case
  when x == 1, x == 2   "one or two"
  when x == 5, x == 6   "five or six"
  else                  "other"
end
```

### D. Ranges in `when`

Use `start..end` (or `start to end`) in a `when` clause to match a numeric range. Both bounds are **inclusive**. No list is allocated — the check is O(1).

```kiwi
fn letter_grade(score)
  return case score
    when 90..100: "A"
    when 80..89:  "B"
    when 70..79:  "C"
    when 60..69:  "D"
    else:         "F"
  end
end

println letter_grade(95)   # A
println letter_grade(72)   # C
println letter_grade(50)   # F
```

Ranges can be mixed freely with literal values and comma-separated lists in the same `case`:

```kiwi
fn classify(n)
  return case n
    when 0:       "zero"
    when 1..9:    "single digit"
    when 10..99:  "double digit"
    else:         "big"
  end
end

fn day_type(d)
  return case d
    when 1..5:  "weekday"
    when 6, 7:  "weekend"
    else:       "invalid"
  end
end
```

Ranges also work with floats and negative bounds:

```kiwi
result = case temp
  when -273.15..0.0:  "below freezing"
  when 0.0..100.0:    "liquid"
  else:               "steam or beyond"
end
```

## 3. `when` – Guard clauses / conditional modifiers

Append `when condition` to many statements for clean early exits.

```kiwi
fn safe_divide(a, b)
  throw "Division by zero" when b == 0
  return a / b
end

for file in files do
  next when exclusions.contains(file)
  process(file)
end

exit 1 when !config.valid
```

Supported with:
- `return … when …`
- `throw … when …`
- `break when …`
- `next when …`
- `exit … when …`
- `x = expr when …` and `x += expr when …` (all assignment forms)
- bare method/function calls: `some_fn() when …`

**Not meaningful** on a bare expression whose value is discarded — the expression evaluates to a value but nothing uses it, so the `when` guard has no observable effect:

```kiwi
x + 1 when flag   # pointless — result is discarded
```

If you want to conditionally execute something, use a method call or an assignment:

```kiwi
log(x + 1) when flag      # ✓ call with side-effect
result = x + 1 when flag  # ✓ assignment (result is null if condition is false)
```

**Lazy evaluation for assignments:** the right-hand side expression is only evaluated when the condition is true. This means it is safe to guard against expensive or potentially failing calls:

```kiwi
for entry in fio::listdir(path, true) do
  total += fio::filesize(entry) when fio::isfile(entry)
end
```

## 4. Ternary operator `?:`

Inline if-else expression.

```kiwi
status = file.exists() ? "found" : "not found"
price_display = discount > 0 ? "${price * (1 - discount)}" : price
```

## Related topics

- Loop-specific controls: [`break`, `next`, `while`, `for`, `repeat`](loops.md)
- Function exit: [`return`](functions.md)
- Error raising: [`throw`](error_handling.md)
- Program termination: [`exit`](keywords.md#exit)
