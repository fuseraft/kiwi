# `sqlite`

The `sqlite` package provides a simple interface for working with SQLite databases. It wraps a native SQLite connection through the `SqliteDb` struct, exposing methods for executing statements and querying data.

```kiwi
import "sqlite"
```

---

## Package Functions

### `sqlite::open(path)`

Opens a SQLite database file and returns a `SqliteDb` instance. Use `":memory:"` for a temporary in-memory database.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | Path to the `.db` file, or `":memory:"` for an in-memory database. |

**Returns**

| Type | Description |
| :--- | :--- |
| `SqliteDb` | An open database connection. |

**Throws**

A `SqliteError` if the database cannot be opened.

**Example**

```kiwi
import "sqlite"

db = sqlite::open("myapp.db")
db.exec("CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY AUTOINCREMENT, msg TEXT)")
db.close()
```

---

## `SqliteDb` Struct

Wraps an open SQLite connection. Obtain an instance via `sqlite::open(path)`.

---

### `close()`

Closes the database connection and releases all resources. Always call this when done.

**Example**

```kiwi
db = sqlite::open(":memory:")
# ... do work ...
db.close()
```

---

### `exec(sql)`

Executes a SQL statement (e.g. `CREATE TABLE`, `INSERT`, `UPDATE`, `DELETE`). For parameterized input, use `exec_params`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `sql` | The SQL statement to execute. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The number of rows affected. |

**Throws**

A `SqliteError` if execution fails.

**Example**

```kiwi
import "sqlite"

db = sqlite::open(":memory:")
db.exec("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER)")
db.exec("INSERT INTO users (name, age) VALUES ('Alice', 30)")
db.close()
```

---

### `exec_params(sql, params)`

Executes a parameterized SQL statement. Parameters are bound positionally using the placeholders `@p1`, `@p2`, etc. Use this instead of string interpolation to avoid SQL injection.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `sql` | The SQL statement with `@p1`, `@p2`, ... placeholders. |
| `list` | `params` | Values to bind in order. Supports `integer`, `float`, `boolean`, `string`, and `null`. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The number of rows affected. |

**Throws**

A `SqliteError` if execution fails.

**Example**

```kiwi
import "sqlite"

db = sqlite::open(":memory:")
db.exec("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER)")
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Alice", 30])
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Bob", 25])
db.close()
```

---

### `query(sql)`

Executes a SQL `SELECT` statement and returns all matching rows.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `sql` | The `SELECT` statement to execute. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of hashmaps, one per row, keyed by column name. |

**Throws**

A `SqliteError` if execution fails.

**Example**

```kiwi
import "sqlite"

db = sqlite::open(":memory:")
db.exec("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER)")
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Alice", 30])

rows = db.query("SELECT * FROM users")
for row in rows do
  name = row["name"]
  age = row["age"]
  println "${name} is ${age} years old"
end

db.close()
```

---

### `query_params(sql, params)`

Executes a parameterized `SELECT` statement and returns all matching rows.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `sql` | The `SELECT` statement with `@p1`, `@p2`, ... placeholders. |
| `list` | `params` | Values to bind in order. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of hashmaps, one per row, keyed by column name. |

**Throws**

A `SqliteError` if execution fails.

**Example**

```kiwi
import "sqlite"

db = sqlite::open(":memory:")
db.exec("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER)")
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Alice", 30])
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Bob", 25])

adults = db.query_params("SELECT name FROM users WHERE age >= @p1", [18])
for row in adults do
  println row["name"]
end

db.close()
```

---

### `last_insert_id()`

Returns the rowid of the most recently inserted row.

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The rowid of the last `INSERT`. |

**Example**

```kiwi
import "sqlite"

db = sqlite::open(":memory:")
db.exec("CREATE TABLE items (id INTEGER PRIMARY KEY AUTOINCREMENT, val TEXT)")
db.exec_params("INSERT INTO items (val) VALUES (@p1)", ["hello"])
println db.last_insert_id()  # prints: 1
db.close()
```

---

### `changes()`

Returns the number of rows modified by the most recent `INSERT`, `UPDATE`, or `DELETE` statement.

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The number of rows changed. |

**Example**

```kiwi
import "sqlite"

db = sqlite::open(":memory:")
db.exec("CREATE TABLE items (id INTEGER PRIMARY KEY AUTOINCREMENT, val TEXT)")
db.exec_params("INSERT INTO items (val) VALUES (@p1)", ["a"])
db.exec_params("INSERT INTO items (val) VALUES (@p1)", ["b"])
db.exec("UPDATE items SET val = 'x'")
println db.changes()  # prints: 2
db.close()
```

---

## Complete Example

```kiwi
import "sqlite"

db = sqlite::open(":memory:")

# Create schema
db.exec("CREATE TABLE products (
  id    INTEGER PRIMARY KEY AUTOINCREMENT,
  name  TEXT    NOT NULL,
  price REAL    NOT NULL,
  stock INTEGER NOT NULL DEFAULT 0
)")

# Insert rows
products = [
  ["Widget",  9.99,  100],
  ["Gadget",  24.99,  50],
  ["Doohickey", 4.49, 200]
]

for p in products do
  db.exec_params("INSERT INTO products (name, price, stock) VALUES (@p1, @p2, @p3)", p)
end

println "Inserted ${db.last_insert_id()} rows"

# Query with filter
cheap = db.query_params("SELECT name, price FROM products WHERE price < @p1 ORDER BY price", [15.0])
println "Affordable products:"
for row in cheap do
  name = row["name"]
  price = row["price"]
  println "  ${name}: $${price}"
end

# Update
db.exec_params("UPDATE products SET stock = stock - @p1 WHERE name = @p2", [10, "Widget"])
println "Stock updated, ${db.changes()} row(s) affected"

db.close()
```
