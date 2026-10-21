# reflector

> Inspect functions and program state at runtime.

The `reflector` package offers runtime introspection: looking up functions by name, examining the program's state, and wrapping callables.

```kiwi
fn square(n)
  n * n
end

f = reflector::getfunc("square")   # look a function up by name
println f(4)                        # 16

println reflector::state().keys()   # ["packages", "structs", "functions", "stack"]
```

`state()` returns the names of the loaded packages, structs and functions, and the variables in each stack frame, which is useful for debugging tools and REPL-style helpers.

## Callable

`Callable` wraps a function together with a name, and exposes its parameters:

```kiwi
adder = Callable.new("adder", do (a, b) => a + b)

println adder.call([3, 7])                           # 10
println adder.parameters().map(do (p) => p["name"])  # ["a", "b"]
```

Every function value also has `.call(arguments)` and `.parameters()` directly; see [Built-in Methods](../reference/builtin-methods.md#functions-and-lambdas).

:::note
`reflector::callstack()` currently returns an empty list.
:::

## Functions

### `reflector::callstack`

```kiwi
reflector::callstack(): list
```

Returns a list of frame names.

**Returns** `list` — A list containing the names of each stack frame in the callstack.

### `reflector::fflags`

```kiwi
reflector::fflags(frame_index: integer = 0): list
```

Returns a list of frame flags at a specified index.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `frame_index` | `integer` | `0` | The frame index. Defaults to 0 (top of the stack). |

**Returns** `list` — The frame flags at specific frame index.

### `reflector::getfunc`

```kiwi
reflector::getfunc(func_name: string): any
```

Returns a specified function as a lambda.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `func_name` | `string` |  | The name of the function. |

**Returns** `any` — The function as a lambda. Returns `null` when not found.

### `reflector::objcontext`

```kiwi
reflector::objcontext(): any
```

Returns a hashmap representing the state of the program.

**Returns** `any` — Returns the current object reference. Returns `null` when not in an object context.

### `reflector::retval`

```kiwi
reflector::retval(): any
```

Returns the current stack frame return value.

**Returns** `any` — The return value at the top of the stack.

### `reflector::state`

```kiwi
reflector::state(): hashmap
```

Returns a hashmap representing the state of the program.

**Returns** `hashmap` —  A hashmap containing four keys: - `packages`: A list containing names of packages imported. - `structs`: The list containing names of structs defined. - `functions`: The list containing names of functions defined. - `stack`: The list of hashmaps containing frame variables. - Each hashmap contains `variables` containing a hashmap of variable names to values.

## `Callable` struct

Inherits from `Hashable`.

A struct representing a callable function or lambda.

### `Callable.new`

```kiwi
Callable.new(name: string, func: lambda)
```

Creates a new Callable instance.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The name of the callable. |
| `func` | `lambda` |  | The lambda representing the callable. |

### `Callable.from_function`

```kiwi
static Callable.from_function(func_name: string): Callable
```

Static method to create a Callable from a function name.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `func_name` | `string` |  | The name of the function. |

**Returns** `Callable` — Returns a new Callable instance wrapping the function.

### `.call`

```kiwi
call(params: list = []): any
```

Invokes the callable with a set of parameters and returns its value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `params` | `list` | `[]` | A list of parameters to pass to the callable. |

**Returns** `any` — The return value of the callable after invocation.

### `.parameters`

```kiwi
parameters(): list
```

Returns a list of hashmaps containing parameter information.

**Returns** `list` —  A list of hashmaps, each containing: - `name`: A string representing the name of the parameter. - `default_value`: The default value. - `type`: If a type annotation was used, then the type name as a string. - `position`: The integer position of the parameter.

### `.returns`

```kiwi
returns(): any
```

Returns the type name of the return type of the callable.

**Returns** `string` — A string containing the type name of the return type.
