---
title: sqlite
description: Query and update SQLite databases.
---

The `sqlite` package works with SQLite databases. `sqlite::open(path)` returns a `SqliteDb`; use the path `":memory:"` for a temporary in-memory database.

```kiwi
db = sqlite::open(":memory:")

db.exec("CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT NOT NULL, price REAL, stock INTEGER DEFAULT 0)")

for p in [["Widget", 9.99, 100], ["Gadget", 24.99, 50], ["Gizmo", 4.49, 200]] do
  db.exec_params("INSERT INTO products (name, price, stock) VALUES (@p1, @p2, @p3)", p)
end
println db.last_insert_id()   # 3

cheap = db.query_params("SELECT name, price FROM products WHERE price < @p1 ORDER BY price", [15.0])
println cheap   # [{"name": "Gizmo", "price": 4.49}, {"name": "Widget", "price": 9.99}]

db.exec_params("UPDATE products SET stock = stock - @p1 WHERE name = @p2", [10, "Widget"])
println db.changes()   # 1

db.close()
```

| Method | Does |
| :-- | :-- |
| `exec(sql)` | Runs a statement that returns no rows. |
| `exec_params(sql, params)` | Runs a statement with parameters. |
| `query(sql)` | Returns the rows of a query as a list of hashmaps. |
| `query_params(sql, params)` | Runs a query with parameters. |
| `last_insert_id()` | The row id of the last insert. |
| `changes()` | The number of rows changed by the last statement. |
| `close()` | Closes the connection. |

## Parameters

Refer to parameters in SQL as `@p1`, `@p2` and so on, and pass their values as a list in the same order. Always use parameters for values that come from outside your program instead of building SQL with string interpolation, which invites SQL injection.

## Errors

A failing statement throws a `SqliteError`:

```kiwi
db = sqlite::open(":memory:")
try
  db.query("SELECT * FROM missing_table")
catch e
  println e["error"]   # SqliteError
end
```

Query results are plain lists of hashmaps, so they work directly with [`table`](/kiwi/stdlib/table/), [`json`](/kiwi/stdlib/json/) and the list methods.

<!-- api: sqlite -->
