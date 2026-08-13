# `env`

The `env` package contains functionality for working with environment variables.

---

## Package Functions

### `all()`

Get environment variables as a hashmap.

**Returns**

| Type | Description |
| :--- | :---|
| `hashmap` | A hashmap representing system environment variables. |

**Example**
```kiwi
import "env"

vars = env::all()
println vars["HOME"]   # /home/user
println vars["PATH"]
```

### `argv()`

Get the command-line arguments supplied to the program, as a hashmap.

**Returns**

| Type | Description |
| :--- | :---|
| `hashmap` | Command-line arguments, one entry per argument. |

Every argument becomes one key. How the *value* is derived depends on the argument's form:

- `-key=value`, `--key=value`, `/key=value` → `argv["key"] == "value"` (see [KVP Command-Line Options](#kvp-command-line-options) below).
- A bare argument with no `=` (e.g. a plain positional value like `foo` or `42`) → self-mapped, `argv["foo"] == "foo"`. This is also true of bare flags like `-v` (no `=`) → `argv["v"] == "v"`, **not** `true`.

**Example**
```kiwi
import "env"

# kiwi myscript.kiwi foo bar -name=world
args = env::argv()
println args   # {"foo": "foo", "bar": "bar", "name": "world"}
```

### `args()`

Get the **raw, ordered** command-line arguments, exactly as passed — duplicates and all.

**Returns**

| Type | Description |
| :--- | :---|
| `list` | The raw command-line arguments, in order, unparsed. |

**Example**
```kiwi
import "env"

# kiwi script.kiwi apple banana apple cherry banana apple
data = env::args()
println data   # ["apple", "banana", "apple", "cherry", "banana", "apple"]
```

**This is the correct way to receive a `script.kiwi item1 item2 ...` style argument list** — use `env::args()`, not `env::argv().keys()`.

> **Why not `argv().keys()`:** `argv()` is a *hashmap* — dictionary keys must be unique, so if the same positional value appears more than once on the command line, the duplicates silently collapse into a single entry before your script ever sees them. `kiwi script.kiwi 5 3 8 3 1` → `argv()` has only 4 keys (`"5"`,`"3"`,`"8"`,`"1"`); `.keys()` from that gives `["5","3","8","1"]`, **not** `["5","3","8","3","1"]`. This breaks anything that depends on the count or position of a repeated value — a word-frequency counter, a sort with duplicate values, a duplicate-finder. `env::args()` has no such limitation because it isn't backed by a hashmap. Only reach for `argv()`/`.keys()` if you specifically want the deduplication (or need the `-key=value` parsing).

Note: `env::args()` returns the *fully raw* tokens, including anything that looks like a flag/option (e.g. `-v`, `--name=world`) — it does no parsing at all. If you need to mix flags/options with positional arguments, use `CliParser` instead — `CliParser.parse()` separates them into named flags/options plus an `"_args"` list of positionals. **Caveat:** `CliParser.parse()`'s `"_args"` is currently derived from `env::argv()`'s keys internally, so it inherits the same duplicate-collapsing behavior described above — don't rely on it if your positional arguments might repeat. See [`cli` package — `CliParser.parse()`](cli.md#parse).

### `opt(_key)`
Get a KVP command-line option value by key.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_key` | The option key.|

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The option value. |

## KVP Command-Line Options

You can pass a named command-line argument in the form of a key-value pair.

```bash
kiwi -key=value
kiwi --key=value
kiwi /key=value
```

You can pull these values using this package.

```kiwi
println(env::opt("key")) # prints: value
```

### `parse_args(args, value_opts)`

Parses a list of command-line argument strings into a hashmap of key-value pairs. Arguments listed in `value_opts` consume the next argument as their value; all other arguments are mapped to `true`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `args` | A list of argument strings. Defaults to `env::args()` (the raw, unparsed argument list) if empty. |
| `list` | `value_opts` | A list of option names that expect a following value argument — match the *raw* token, dashes included (e.g. `"--output"`, not `"output"`). |

**Returns**

| Type | Description |
| :--- | :---|
| `hashmap` | A hashmap of parsed options. |

**Example**
```kiwi
# kiwi myscript.kiwi --output result.txt --verbose
cfg = env::parse_args([], ["--output"])
println cfg["--output"]   # prints: result.txt
println cfg["--verbose"]  # prints: true
```

Only the space-separated form (`--output result.txt`) is understood — `parse_args` walks raw tokens and does not split on `=`, so `--output=result.txt` is treated as one opaque flag token (`cfg["--output=result.txt"] = true`) rather than a key/value pair. Use the space-separated form, or pre-split `=`-joined tokens yourself before calling `parse_args`.

**Throws**
If a value option is listed in `value_opts` but no following argument exists.

---

### `get(_varname)`

Get an environment variable.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_varname` | The environment variable name to retrieve. |

**Returns**

| Type | Description |
| :--- | :---|
| `string` | The environment variable value, an empty string if not found. |

**Example**
```kiwi
import "env"

home = env::get("HOME")
println home   # /home/user
```

### `set(_varname, _varvalue)`

Set an environment variable.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_varname` | The environment variable name to set. |
| `string` | `_varvalue` | The environment variable value. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | `true` on success. |

**Example**
```kiwi
import "env"

env::set("APP_ENV", "production")
println env::get("APP_ENV")   # production
```

### `bin()`

Returns a string containing the path to the Kiwi executable.

**Returns**

| Type | Description |
| :--- | :---|
| `string` | Path to Kiwi. |

**Example**
```kiwi
import "env"

println env::bin()   # /usr/local/bin/kiwi
```

### `lib()`

Returns a string containing the path to the Kiwi Standard Library.

**Returns**

| Type | Description |
| :--- | :---|
| `string` | Path to Kiwi Standard Library. |

**Example**
```kiwi
import "env"

println env::lib()   # /usr/local/lib/kiwi
```