# dotenv

> Load .env files into the process environment.

The `dotenv` package reads `KEY=VALUE` files, conventionally named `.env`, and loads them into the environment so the rest of the program can read them with [`env::get`](env.md).

```kiwi
dotenv::load()                 # reads ./.env
port = env::get("PORT")

dotenv::load("config/.env.production", false)   # don't overwrite existing variables
```

`load` returns the values it read. `parse_file` and `parse` return the values without touching the environment:

```kiwi
settings = dotenv::parse("PORT=8080\nNAME=\"kiwi app\"  # display name\n# a comment")
println settings   # {"PORT": "8080", "NAME": "kiwi app"}
```

Values are always strings.

## File format

| Form | Example | Notes |
| :-- | :-- | :-- |
| Plain | `KEY=value` | Surrounding whitespace is trimmed. |
| Double quoted | `KEY="hello world"` | Keeps whitespace; supports `\"`, `\\`, `\n`, `\t` and `\r`. |
| Single quoted | `KEY='literal $value'` | Taken literally. |
| Inline comment | `KEY=value  # note` | Everything from ` #` on is ignored. |
| `export` prefix | `export KEY=value` | The `export` is ignored. |
| Comment | `# comment` | Ignored, as are blank lines. |

## Functions

### `dotenv::parse`

```kiwi
dotenv::parse(content: string): hashmap
```

Parses a .env-formatted string into a hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `content` | `string` |  | The dotenv-formatted string to parse. |

**Returns** `hashmap` — A hashmap of key-value pairs.

### `dotenv::parse_file`

```kiwi
dotenv::parse_file(file_path: string = ".env"): hashmap
```

Parses a .env file into a hashmap without modifying the environment.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `file_path` | `string` | `".env"` | The path to the .env file. Defaults to `".env"`. |

**Returns** `hashmap` — A hashmap of key-value pairs.

**Throws** `string` — If the file does not exist.

### `dotenv::load`

```kiwi
dotenv::load(file_path: string = ".env", overwrite: boolean = true): hashmap
```

Loads a .env file and sets each entry as an environment variable.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `file_path` | `string` | `".env"` | The path to the .env file. Defaults to `".env"`. |
| `overwrite` | `boolean` | `true` | When `true`, existing environment variables are overwritten. Defaults to `true`. |

**Returns** `hashmap` — A hashmap of the loaded key-value pairs.

**Throws** `string` — If the file does not exist.
