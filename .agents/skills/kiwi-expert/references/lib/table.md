# table

> Filter, join, group and display lists of hashmaps.

The `table` package queries lists of hashmaps (rows) the way you would a database table. Wrap rows with `table::from`, chain operations, and finish with a method that returns a result. Rows from [`csv`](csv.md), [`json`](json.md) and [`sqlite`](sqlite.md) all work.

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

## Functions

### `table::from`

```kiwi
table::from(rows: list): Table
```

Creates a new Table from a list of hashmaps.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `rows` | `list` |  | The source data. |

**Returns** `Table` — A new Table instance.

## `Table` struct

An in-memory table of rows (list of hashmaps) with a fluent transform API.

```kiwi
# Filter, project, sort
result = table::from(rows)
  .where(do (r) => r["active"] == true end)
  .select(["id", "name", "score"])
  .order_by("score", false)
  .limit(10)
  .to_list()

# Join two datasets
result = table::from(users)
  .inner_join(orders, "id", "user_id")
  .select(["name", "product", "qty"])
  .to_list()

# Group and aggregate
result = table::from(orders)
  .group_by("product")
  .agg({
    "total": do (rows) => rows.map(do (r) => r["qty"] end).sum() end,
    "n":     do (rows) => rows.size() end
  })
  .order_by("total", false)
  .to_list()
```

### `Table.new`

```kiwi
Table.new(rows: list)
```

Creates a new Table wrapping a list of hashmaps.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `rows` | `list` |  | The source data — a list of hashmaps. |

### `.where`

```kiwi
where(predicate: lambda): Table
```

Keeps only rows for which `predicate(row)` is truthy.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `predicate` | `lambda` |  | A lambda accepting a row hashmap. |

**Returns** `Table` — self, for chaining.

### `.select`

```kiwi
select(keys: list): Table
```

Projects each row to only the specified keys.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `keys` | `list` |  | The field names to retain. |

**Returns** `Table` — self, for chaining.

### `.rename`

```kiwi
rename(old_name: string, new_name: string): Table
```

Renames a field in every row.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `old_name` | `string` |  | The existing field name. |
| `new_name` | `string` |  | The replacement field name. |

**Returns** `Table` — self, for chaining.

### `.add_field`

```kiwi
add_field(name: string, compute: lambda): Table
```

Adds a computed field to every row.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The new field name. |
| `compute` | `lambda` |  | A lambda accepting a row and returning the field value. |

**Returns** `Table` — self, for chaining.

### `.drop`

```kiwi
drop(keys: list): Table
```

Removes the specified fields from every row.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `keys` | `list` |  | The field names to drop. |

**Returns** `Table` — self, for chaining.

### `.map`

```kiwi
map(transform: lambda): Table
```

Applies a custom transform lambda to every row.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `transform` | `lambda` |  | A lambda accepting a row hashmap and returning a hashmap. |

**Returns** `Table` — self, for chaining.

### `.order_by`

```kiwi
order_by(field: string, asc: boolean = true): Table
```

Sorts rows by a field value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `field` | `string` |  | The field name to sort by. |
| `asc` | `boolean` | `true` | `true` for ascending (default), `false` for descending. |

**Returns** `Table` — self, for chaining.

### `.limit`

```kiwi
limit(n: integer): Table
```

Keeps only the first `n` rows.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` |  | Maximum number of rows to keep. |

**Returns** `Table` — self, for chaining.

### `.offset`

```kiwi
offset(n: integer): Table
```

Skips the first `n` rows.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` |  | Number of rows to skip. |

**Returns** `Table` — self, for chaining.

### `.distinct`

```kiwi
distinct(): Table
```

Removes duplicate rows (compares all fields).

**Returns** `Table` — self, for chaining.

### `.inner_join`

```kiwi
inner_join(other: list, left_key: string, right_key: string): Table
```

Keeps only rows that have a matching row in `other`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `other` | `list` |  | The right-side list of hashmaps. |
| `left_key` | `string` |  | The join field on the left (this table). |
| `right_key` | `string` |  | The join field on the right (`other`). |

**Returns** `Table` — self, for chaining.

### `.left_join`

```kiwi
left_join(other: list, left_key: string, right_key: string): Table
```

Keeps all left rows; right-side fields are absent when there is no match.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `other` | `list` |  | The right-side list of hashmaps. |
| `left_key` | `string` |  | The join field on the left (this table). |
| `right_key` | `string` |  | The join field on the right (`other`). |

**Returns** `Table` — self, for chaining.

### `.right_join`

```kiwi
right_join(other: list, left_key: string, right_key: string): Table
```

Keeps all right rows; left-side fields are absent when there is no match.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `other` | `list` |  | The right-side list of hashmaps. |
| `left_key` | `string` |  | The join field on the left (this table). |
| `right_key` | `string` |  | The join field on the right (`other`). |

**Returns** `Table` — self, for chaining.

### `.full_join`

```kiwi
full_join(other: list, left_key: string, right_key: string): Table
```

Keeps all rows from both sides; unmatched rows have fields only from their side.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `other` | `list` |  | The right-side list of hashmaps. |
| `left_key` | `string` |  | The join field on the left (this table). |
| `right_key` | `string` |  | The join field on the right (`other`). |

**Returns** `Table` — self, for chaining.

### `.group_by`

```kiwi
group_by(field: string): Table
```

Groups rows by the unique values of `field`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `field` | `string` |  | The field to group by. |

**Returns** `Table` — self, for chaining.

### `.agg`

```kiwi
agg(specs: hashmap): Table
```

Collapses groups into one row per group using aggregate functions.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `specs` | `hashmap` |  | `{ "out_field": do (rows) => ... end, ... }` |

**Returns** `Table` — self, for chaining.

**Throws** `string` — If called without a preceding `group_by()`.

```kiwi
table::from(orders)
  .group_by("product")
  .agg({
    "total": do (rows) => rows.map(do (r) => r["qty"] end).sum() end,
    "n":     do (rows) => rows.size() end
  })
  .to_list()
```

### `.to_list`

```kiwi
to_list(): list
```

Returns the current rows as a plain list.

**Returns** `list` — List of row hashmaps.

### `.to_hashmap`

```kiwi
to_hashmap(key_field: string): hashmap
```

Returns a hashmap keyed by the values of `key_field`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key_field` | `string` |  | The field whose value becomes each hashmap key. |

**Returns** `hashmap` — A hashmap mapping key values to row hashmaps.

### `.pluck`

```kiwi
pluck(field: string): list
```

Extracts a single field's values as a flat list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `field` | `string` |  | The field name to extract. |

**Returns** `list` — A list of the field values.

### `.first`

```kiwi
first()
```

Returns the first row, or `null` if the table is empty.

**Returns** `hashmap|null` — The first row, or null.

### `.count`

```kiwi
count(): integer
```

Returns the number of rows.

**Returns** `integer` — Row count.

### `.display`

```kiwi
display(max_width: integer = 0): Table
```

Prints the table as a formatted ASCII grid and returns self.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `max_width` | `integer` | `0` | Maximum column width in characters (0 = unlimited). Values longer than `max_width` are truncated with `…`. |

**Returns** `Table` — self, for chaining.

```kiwi
table::from(rows)
  .where(do (r) => r["active"] end)
  .order_by("name")
  .display()

# Cap every column at 30 characters
table::from(rows).display(30)
```
