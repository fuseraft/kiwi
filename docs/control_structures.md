# Control Flow in Kiwi

Kiwi offers a compact but very expressive set of control structures.

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

### !. Condition-only `case` (when conditions are unrelated)

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
