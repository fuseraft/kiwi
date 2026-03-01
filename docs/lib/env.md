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

Get the list of command-line arguments supplied to the program.

**Returns**

| Type | Description |
| :--- | :---|
| `list` | A list of command-line arguments. |

**Example**
```kiwi
import "env"

# kiwi myscript.kiwi foo bar
args = env::argv()
println args   # { foo: "", bar: "" }
```

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
| `list` | `args` | A list of argument strings. Defaults to `env::argv().keys()` if empty. |
| `list` | `value_opts` | A list of option names that expect a following value argument. |

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