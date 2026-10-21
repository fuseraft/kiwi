# sqlite

> Query and update SQLite databases.

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

Query results are plain lists of hashmaps, so they work directly with [`table`](table.md), [`json`](json.md) and the list methods.

## Functions

### `sqlite::open`

```kiwi
sqlite::open(path: string): SqliteDb
```

Opens a SQLite database and returns a `SqliteDb` instance.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the `.db` file. Use `":memory:"` for an in-memory database. |

**Returns** `SqliteDb` — An open database connection.

**Throws** `string` — If the database cannot be opened.

```kiwi
db = sqlite::open("myapp.db")
db.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)")
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Alice", 30])
rows = db.query("SELECT * FROM users")
db.close()
```

### `sqlite::__open__`

```kiwi
sqlite::__open__(path: string)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  |  |

### `sqlite::__close__`

```kiwi
sqlite::__close__(conn)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |

### `sqlite::__exec__`

```kiwi
sqlite::__exec__(conn, sql: string): integer
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |
| `sql` | `string` |  |  |

**Returns** `integer`

### `sqlite::__exec_params__`

```kiwi
sqlite::__exec_params__(conn, sql: string, params: list): integer
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |
| `sql` | `string` |  |  |
| `params` | `list` |  |  |

**Returns** `integer`

### `sqlite::__query__`

```kiwi
sqlite::__query__(conn, sql: string): list
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |
| `sql` | `string` |  |  |

**Returns** `list`

### `sqlite::__query_params__`

```kiwi
sqlite::__query_params__(conn, sql: string, params: list): list
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |
| `sql` | `string` |  |  |
| `params` | `list` |  |  |

**Returns** `list`

### `sqlite::__last_insert_id__`

```kiwi
sqlite::__last_insert_id__(conn): integer
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |

**Returns** `integer`

### `sqlite::__changes__`

```kiwi
sqlite::__changes__(conn): integer
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `conn` | `any` |  |  |

**Returns** `integer`

## `SqliteDb` struct

A wrapper around an open SQLite database connection.

### `SqliteDb.new`

```kiwi
SqliteDb.new(path: string)
```

Opens a connection to the SQLite database at the given path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the `.db` file. Use `":memory:"` for an in-memory database. |

### `.close`

```kiwi
close()
```

Closes the database connection and releases resources.

### `.exec`

```kiwi
exec(sql: string): integer
```

Executes a SQL statement (INSERT, UPDATE, DELETE, CREATE, etc.).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sql` | `string` |  | The SQL statement to execute. |

**Returns** `integer` — The number of rows affected.

### `.exec_params`

```kiwi
exec_params(sql: string, params: list): integer
```

Executes a parameterized SQL statement with positional parameters.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sql` | `string` |  | The SQL statement using `@p1`, `@p2`, ... placeholders. |
| `params` | `list` |  | A list of values to bind in order. |

**Returns** `integer` — The number of rows affected.

```kiwi
db.exec_params("INSERT INTO users (name, age) VALUES (@p1, @p2)", ["Alice", 30])
```

### `.query`

```kiwi
query(sql: string): list
```

Executes a SQL query and returns all rows as a list of hashmaps.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sql` | `string` |  | The SELECT statement to execute. |

**Returns** `list` — A list of hashmaps, one per row, keyed by column name.

### `.query_params`

```kiwi
query_params(sql: string, params: list): list
```

Executes a parameterized SQL query with positional parameters.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sql` | `string` |  | The SELECT statement using `@p1`, `@p2`, ... placeholders. |
| `params` | `list` |  | A list of values to bind in order. |

**Returns** `list` — A list of hashmaps, one per row, keyed by column name.

```kiwi
db.query_params("SELECT * FROM users WHERE age > @p1", [18])
```

### `.last_insert_id`

```kiwi
last_insert_id(): integer
```

Returns the rowid of the last successful INSERT.

**Returns** `integer` — The rowid of the most recently inserted row.

### `.changes`

```kiwi
changes(): integer
```

Returns the number of rows changed by the last INSERT, UPDATE, or DELETE.

**Returns** `integer` — The number of rows changed.
