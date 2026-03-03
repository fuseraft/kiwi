# `dotenv`

The `dotenv` package loads `.env` files into the process environment and provides utilities for parsing dotenv-formatted content.

---

## Syntax Reference

A `.env` file contains `KEY=VALUE` pairs, one per line. The following syntax is supported:

| Form | Example | Notes |
| :--- | :--- | :--- |
| Plain | `KEY=value` | Whitespace around value is trimmed. |
| Double-quoted | `KEY="hello world"` | Preserves whitespace; supports `\"`, `\\`, `\n`, `\t`, `\r`. |
| Single-quoted | `KEY='literal value'` | Literal — no escape processing. |
| Inline comment | `KEY=value  # note` | Everything from ` #` onward is stripped. |
| Export prefix | `export KEY=value` | `export ` is stripped; otherwise treated normally. |
| Comment line | `# this is a comment` | Entire line is ignored. |
| Blank line | | Ignored. |

---

## Package Functions

### `parse(content)`

Parses a dotenv-formatted string into a hashmap without modifying the environment.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `content` | A dotenv-formatted string. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap of key-value pairs parsed from `content`. |

**Example**

```kiwi
import "dotenv"

content = "HOST=localhost\nPORT=5432\nDEBUG=true  # dev only\n"
cfg = dotenv::parse(content)

println cfg["HOST"]   # localhost
println cfg["PORT"]   # 5432
println cfg["DEBUG"]  # true
```

---

### `parse_file(file_path)`

Parses a `.env` file into a hashmap without modifying the environment.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `file_path` | Path to the `.env` file. Defaults to `".env"`. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap of key-value pairs parsed from the file. |

**Throws**

A `string` error if the file does not exist.

**Example**

```kiwi
import "dotenv"

# Read variables without touching the environment
cfg = dotenv::parse_file(".env.test")

println cfg["DB_URL"]
println cfg["API_KEY"]
```

---

### `load(file_path, overwrite)`

Parses a `.env` file and sets each key as an environment variable via `env::set`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `file_path` | Path to the `.env` file. Defaults to `".env"`. |
| `boolean` | `overwrite` | When `true`, existing environment variables are overwritten. Defaults to `true`. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap of the key-value pairs that were loaded. |

**Throws**

A `string` error if the file does not exist.

**Example**

```kiwi
import "dotenv"

# Load .env into the environment (overwrites existing vars)
dotenv::load()

println env::get("APP_ENV")   # production

# Load a different file, keeping existing env vars intact
dotenv::load(".env.local", false)
```
