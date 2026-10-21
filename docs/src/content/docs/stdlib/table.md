---
title: table
description: Filter, join, group and display lists of hashmaps.
---

The `table` package queries lists of hashmaps (rows) the way you would a database table. Wrap rows with `table::from`, chain operations, and finish with a method that returns a result. Rows from [`csv`](/kiwi/stdlib/csv/), [`json`](/kiwi/stdlib/json/) and [`sqlite`](/kiwi/stdlib/sqlite/) all work.

```kiwi
users = [
  {id: 1, name: "Ada", active: true, score: 91},
  {id: 2, name: "Grace", active: true, score: 87},
  {id: 3, name: "Linus", active: false, score: 75}
]

top = table::from(users)
  .where(do (r) => r["active"] end)
  .select(["name", "score"])
  .order_by("score", false)      # descending
  .to_list()

println top   # [{"name": "Ada", "score": 91}, {"name": "Grace", "score": 87}]
```

Each step returns a new table; the source rows are not modified.

## Transforming

| Method | Does |
| :-- | :-- |
| `where(predicate)` | Keeps rows for which the predicate is truthy. |
| `select(fields)`, `drop(fields)` | Keeps or removes fields. |
| `rename(old, new)` | Renames a field. |
| `add_field(name, compute)` | Adds a field computed from each row. |
| `map(transform)` | Replaces each row with the transform's result. |
| `order_by(field, ascending)` | Sorts by a field; ascending by default. |
| `limit(n)`, `offset(n)` | Keeps the first `n` rows, or skips them. |
| `distinct()` | Removes duplicate rows. |

```kiwi
graded = table::from([{name: "Ada", score: 91}, {name: "Linus", score: 75}])
  .add_field("grade", do (r) => r["score"] >= 90 ? "A" : "B" end)
  .pluck("grade")

println graded   # ["A", "B"]
```

## Joining

`inner_join`, `left_join`, `right_join` and `full_join` take the other rows, the key field on this side, and the key field on the other side. Joined rows merge the fields of both; if both sides have a field with the same name, `rename` one first.

```kiwi
users = [{id: 1, name: "Ada"}, {id: 2, name: "Grace"}, {id: 3, name: "Linus"}]
orders = [
  {user_id: 1, product: "kiwi", qty: 3},
  {user_id: 2, product: "mango", qty: 1}
]

println table::from(users)
  .inner_join(orders, "id", "user_id")
  .select(["name", "product"])
  .to_list()
# [{"name": "Ada", "product": "kiwi"}, {"name": "Grace", "product": "mango"}]

# Users without orders: left join, then keep rows that gained no order fields.
println table::from(users)
  .left_join(orders, "id", "user_id")
  .where(do (r) => !r.keys().contains("product") end)
  .pluck("name")
# ["Linus"]
```

## Grouping

`group_by(field)` followed by `agg(specs)` produces one row per group. Each spec is a lambda that receives the group's rows:

```kiwi
orders = [
  {region: "west", qty: 3},
  {region: "east", qty: 1},
  {region: "west", qty: 2}
]

println table::from(orders)
  .group_by("region")
  .agg({
    "orders": do (rows) => rows.size() end,
    "units": do (rows) => rows.map(do (r) => r["qty"] end).sum() end
  })
  .to_list()
# [{"region": "west", "orders": 2, "units": 5}, {"region": "east", "orders": 1, "units": 1}]
```

## Results

| Method | Returns |
| :-- | :-- |
| `to_list()` | The rows. |
| `pluck(field)` | One field's values. |
| `first()` | The first row, or `null`. |
| `count()` | The number of rows. |
| `to_hashmap(field)` | A hashmap from each row's `field` value, as a string, to the row. |
| `display(max_width)` | Prints the rows as a table and returns the table, so it can sit mid-chain while debugging. |

```kiwi
table::from([{id: 1, name: "Ada"}, {id: 2, name: "Grace"}]).display()
```

```text
+----+-------+
| id | name  |
+----+-------+
| 1  | Ada   |
| 2  | Grace |
+----+-------+
(2 row(s))
```

<!-- api: table -->
