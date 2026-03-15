# `table`

The `table` package provides an in-memory tabular data pipeline for filtering, projecting, joining, grouping, and aggregating lists of hashmaps. It is automatically loaded by the Kiwi runtime — no explicit import is needed.

Every transform method returns `self`, so operations can be chained freely. Call a terminal method to retrieve results.

---

## Package Functions

### `table::from(rows)`

Creates a new `Table` wrapping a list of hashmaps.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `rows` | A list of hashmaps — typically the output of a `csv::pipeline()` or `json::pipeline()` call. |

**Returns**

| Type | Description |
| :--- | :--- |
| `Table` | A new Table instance. |

---

## `Table`

### Transform methods

All transform methods return `self` for chaining.

#### `.where(predicate)`

Keeps only rows for which `predicate(row)` is truthy.

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `predicate` | A lambda accepting a row hashmap. |

#### `.select(keys)`

Projects each row to only the specified fields.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `keys` | The field names to retain. |

#### `.rename(old_name, new_name)`

Renames a field in every row.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `old_name` | The existing field name. |
| `string` | `new_name` | The replacement field name. |

#### `.add_field(name, compute)`

Adds a computed field to every row.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | The new field name. |
| `lambda` | `compute` | A lambda accepting a row and returning the new field's value. |

#### `.drop(keys)`

Removes the specified fields from every row.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `keys` | The field names to remove. |

#### `.map(transform)`

Applies a custom transform to every row. The lambda receives a row hashmap and must return the new row.

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `transform` | A lambda accepting a row hashmap and returning a hashmap. |

#### `.order_by(field, asc?)`

Sorts rows by a field value.

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `field` | The field name to sort by. | |
| `boolean` | `asc` | `true` for ascending, `false` for descending. | `true` |

#### `.limit(n)`

Keeps only the first `n` rows.

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `n` | Maximum number of rows to keep. |

#### `.offset(n)`

Skips the first `n` rows.

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `n` | Number of rows to skip. |

#### `.distinct()`

Removes duplicate rows by comparing all fields.

---

### Join methods

All join methods return `self`. When a left and right row are merged, right-side fields overwrite left-side fields on name collision. Use `.rename()` or `.select()` before joining to avoid unwanted overwrites.

#### `.inner_join(other, left_key, right_key)`

Keeps only rows that have at least one matching row in `other`.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `other` | The right-side list of hashmaps. |
| `string` | `left_key` | The join field on this table. |
| `string` | `right_key` | The join field on `other`. |

#### `.left_join(other, left_key, right_key)`

Keeps all rows from this table. Rows with no match in `other` are included without any right-side fields.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `other` | The right-side list of hashmaps. |
| `string` | `left_key` | The join field on this table. |
| `string` | `right_key` | The join field on `other`. |

#### `.right_join(other, left_key, right_key)`

Keeps all rows from `other`. Rows with no match in this table are included without any left-side fields.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `other` | The right-side list of hashmaps. |
| `string` | `left_key` | The join field on this table. |
| `string` | `right_key` | The join field on `other`. |

#### `.full_join(other, left_key, right_key)`

Keeps all rows from both sides. Unmatched rows carry only their own side's fields.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `other` | The right-side list of hashmaps. |
| `string` | `left_key` | The join field on this table. |
| `string` | `right_key` | The join field on `other`. |

#### Anti-join pattern

To find rows in this table with **no** match in another table (equivalent to `LEFT JOIN … WHERE right.col IS NULL`), use `.left_join()` followed by `.where()` checking for the absence of a right-side field:

```kiwi
# Users who have placed no orders
table::from(users)
  .left_join(orders, "id", "user_id")
  .where(do (r) => !r.keys().contains("user_id") end)
  .to_list()
```

After a left join, unmatched rows retain only their original fields — so any field that only exists on the right side will be absent.

---

### Grouping & Aggregation

#### `.group_by(field)`

Groups rows by the unique values of `field`. Must be followed by `.agg()`.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `field` | The field to group by. |

#### `.agg(specs)`

Collapses groups into one row per group. `specs` is a hashmap mapping output field names to aggregate lambdas. Each lambda receives the list of rows in its group and returns a value. The group-by field is automatically included in the output.

**Throws** — if called without a preceding `.group_by()`.

| Type | Name | Description |
| :--- | :--- | :--- |
| `hashmap` | `specs` | `{ "out_field": do (rows) => … end, … }` |

```kiwi
table::from(orders)
  .group_by("product")
  .agg({
    "count":   do (rows) => rows.size() end,
    "total":   do (rows) => rows.map(do (r) => r["qty"] end).sum() end,
    "avg_qty": do (rows) => rows.map(do (r) => r["qty"] end).sum() / rows.size() end
  })
  .to_list()
```

---

### Terminal methods

#### `.to_list()`

Returns the current rows as a plain list of hashmaps.

#### `.to_hashmap(key_field)`

Returns a hashmap keyed by the values of `key_field`. If multiple rows share the same key value, the last one wins.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `key_field` | The field whose value becomes each hashmap key. |

**Returns** `hashmap`

#### `.pluck(field)`

Extracts a single field's values as a flat list.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `field` | The field name to extract. |

**Returns** `list`

#### `.first()`

Returns the first row, or `null` if the table is empty.

**Returns** `hashmap | null`

#### `.count()`

Returns the number of rows.

**Returns** `integer`

#### `.display()`

Prints the table as a formatted ASCII grid and returns `self` for continued chaining.

Columns are derived from the first row's keys. Each column is wide enough to fit its header and all its values.

**Returns** `Table` (self)

```kiwi
table::from(rows)
  .order_by("score", false)
  .display()
# +----+-------+-------+
# | id | name  | score |
# +----+-------+-------+
# |  1 | Alice |     9 |
# …
# (4 row(s))
```

---

## Examples

### Filter, project, sort

```kiwi
result = table::from(users)
  .where(do (r) => r["active"] == true end)
  .select(["id", "name", "score"])
  .order_by("score", false)
  .limit(10)
  .to_list()
```

### Computed field

```kiwi
table::from(products)
  .add_field("discounted", do (r) => r["price"] * 0.9 end)
  .display()
```

### Inner join

```kiwi
result = table::from(users)
  .inner_join(orders, "id", "user_id")
  .select(["name", "product", "qty"])
  .to_list()
```

### Rename before join to avoid field collision

```kiwi
# Both users and products have a "name" field.
# Rename one before joining to keep both.
product_lookup = table::from(products)
  .rename("name", "product_name")
  .to_list()

result = table::from(orders)
  .inner_join(product_lookup, "product_id", "id")
  .select(["order_id", "product_name", "qty"])
  .to_list()
```

### Group and aggregate

```kiwi
stats = table::from(orders)
  .group_by("region")
  .agg({
    "orders":     do (rows) => rows.size() end,
    "total_qty":  do (rows) => rows.map(do (r) => r["qty"] end).sum() end,
    "total_rev":  do (rows) => rows.map(do (r) => r["revenue"] end).sum() end
  })
  .order_by("total_rev", false)
  .display()
```

### Anti-join: rows with no match

```kiwi
# Find users who have never placed an order
no_orders = table::from(users)
  .left_join(orders, "id", "user_id")
  .where(do (r) => !r.keys().contains("user_id") end)
  .pluck("name")
```

### Fast lookup with `to_hashmap`

```kiwi
user_map = table::from(users).to_hashmap("id")
u = user_map["42"]
println u["name"]
```

### Debugging mid-chain with `display`

```kiwi
table::from(rows)
  .where(do (r) => r["active"] end)
  .display()        # inspect here without breaking the chain
  .pluck("id")
```
