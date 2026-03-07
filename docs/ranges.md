# Ranges

Ranges in Kiwi can be used to define a range of integers. Use the `to` keyword or `..` to denote a range.

---

## As a List

When used inside `[` `]`, a range is immediately evaluated as a [List](./lists.md).

### Syntax

**`[`** *start* **`to`** *end* **`]`**
**`[`** *start* **`..`** *end* **`]`**

### Example

```kiwi
println [1 to 5]
# Prints: [1, 2, 3, 4, 5]

# Build a list of numbers from 1 to 10 inclusive, then reverse it.
println [1 to 10][::-1]
# Prints: [10, 9, 8, 7, 6, 5, 4, 3, 2, 1]

r = [20 to 18]
println r
# Prints: [20, 19, 18]
```

## In `case when`

Ranges can appear directly in `when` clauses without brackets. The match is O(1) — no list is constructed.

```kiwi
fn describe(n)
  return case n
    when 1..9:   "single digit"
    when 10..99: "double digit"
    else:        "other"
  end
end

println describe(7)    # single digit
println describe(42)   # double digit
println describe(100)  # other
```

Both `..` and `to` are accepted, bounds are **inclusive**, and ranges mix freely with literal values in the same `case`:

```kiwi
fn day_type(d)
  return case d
    when 1..5:  "weekday"
    when 6, 7:  "weekend"
    else:       "invalid"
  end
end
```

See [Control Structures](control_structures.md) for full details.