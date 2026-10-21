# env

> Environment variables and command-line arguments.

The `env` package reads and writes environment variables and gives access to the arguments a script was started with.

## Environment variables

```kiwi
env::set("APP_MODE", "test")
println env::get("APP_MODE")         # test
println env::get("NOT_SET").empty()  # true: missing variables read as ""
println typeof(env::all())           # hashmap
```

`env::set` is unavailable in [safe mode](../tools/cli.md#safe-mode). To load variables from a `.env` file, see [`dotenv`](dotenv.md).

## Command-line arguments

| Function | Returns |
| :-- | :-- |
| `env::args()` | The raw arguments, in order. |
| `env::argv()` | The arguments as a hashmap: `-key=value` becomes `key: "value"`, `--flag` becomes `flag: "true"`, and other arguments map to themselves. |
| `env::opt(key)` | The value of a `-key=value` argument. |
| `env::parse_args(args, value_options)` | A hashmap of flags, where options listed in `value_options` take the following argument as their value. |

```kiwi
config = env::parse_args(["--mode", "fast", "-v"], ["--mode"])
println config   # {"--mode": "fast", "-v": true}
```

See [Passing arguments to scripts](../tools/cli.md#passing-arguments-to-scripts) for examples, and [`cli::CliParser`](cli.md#parsing-arguments) for a fuller argument parser.

## Functions

### `env::parse_args`

```kiwi
env::parse_args(args: list = [], value_opts: list = []): hashmap
```

Parses command-line arguments into a key-value hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `args` | `list` | `[]` | The list of argument strings to parse (default: the raw `args()` list). |
| `value_opts` | `list` | `[]` | Options that expect a following value argument (default `[]`). |

**Returns** `hashmap` — A hashmap where flag options map to `true` and value options map to their argument.

**Throws** `string` — If a value option is found with no following argument.

### `env::argv`

```kiwi
env::argv(): hashmap
```

Get the hashmap of command-line arguments.

**Returns** `hashmap` — A hashmap of command-line arguments.

### `env::args`

```kiwi
env::args(): list
```

Get the raw, ordered list of command-line arguments, exactly as passed (duplicates preserved).

**Returns** `list` — The raw command-line arguments, in order.

### `env::opt`

```kiwi
env::opt(_key: string): string
```

Get a named option value by key.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_key` | `string` |  | The option key. |

**Returns** `string` — The option value.

### `env::all`

```kiwi
env::all(): hashmap
```

Get all environment variables as a hashmap.

**Returns** `hashmap` — A hashmap of all environment variables.

### `env::get`

```kiwi
env::get(_var_name: string): string
```

Get an environment variable.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_var_name` | `string` |  | The environment variable name. |

**Returns** `string` — The environment variable value, or an empty string if not found.

### `env::set`

```kiwi
env::set(_var_name: string, _var_value: string): boolean
```

Set an environment variable.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_var_name` | `string` |  | The environment variable name. |
| `_var_value` | `string` |  | The environment variable value. |

**Returns** `boolean` — `true` on success, `false` on failure.

### `env::bin`

```kiwi
env::bin(): string
```

Get the path to the Kiwi binary.

**Returns** `string` — The path to the Kiwi binary.

### `env::lib`

```kiwi
env::lib(): string
```

Get the path to the Kiwi Standard Library.

**Returns** `string` — The path to the Kiwi Standard Library directory.
