# Ranges

Ranges in Kiwi can be used to define a range of integers. Use the `to` keyword or `..` to denote a range.

Ranges are immediately evaluated as [Lists](./lists.md).

### Syntax

**`[`** *start* **`to`** *end* **`]`**

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