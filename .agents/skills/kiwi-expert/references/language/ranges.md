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

> **Pitfall — loop bounds that can invert:** `[a to b]` is never empty and never errors when `b < a` — it just counts *down*. This is easy to hit by accident in a shrinking loop bound like `for i in [0 to n-2] do ... end`: as soon as `n` reaches `1`, `n-2` is `-1`, and `[0 to -1]` silently becomes the 2-element descending list `[0, -1]` rather than an empty range — the loop body then runs with `i = -1`, which typically blows up as an out-of-bounds index rather than failing fast at the boundary. If a loop bound is computed and could reach 0 or go negative, guard the loop's entry condition explicitly (e.g. `while cond and n > 1 do ...`) rather than trusting `[0 to n-2]` to become empty on its own.

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