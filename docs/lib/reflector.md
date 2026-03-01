# `reflector`

The `reflector` package contains functionality for reflection in Kiwi.

---

## Package Functions

### `callstack()`
Returns a list of frame names.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list containing the names of each stack frame in the callstack. |

**Example**

```kiwi
import "reflector"

fn inner()
  println reflector::callstack()
end

fn middle()
  inner()
end

fn outer()
  middle()
end

outer()
# => ["inner", "middle", "outer", "__global__"]
# Frames are listed innermost-first (top of stack first).
```

---

### `fflags(frame_index)`
Returns a list of frame flags at a specified index.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `frame_index` | The frame index. Defaults to 0 (top of the stack). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | The frame flags at specific frame index. |

**Example**

```kiwi
import "reflector"

fn check_flags()
  # Frame 0 is the current frame (check_flags itself)
  flags = reflector::fflags(0)
  println "Flags at frame 0: ${flags}"

  # Frame 1 is the caller
  caller_flags = reflector::fflags(1)
  println "Flags at frame 1: ${caller_flags}"
end

check_flags()
```

---

### `getfunc(func_name)`
Returns a specified function as a lambda.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `func_name` | The name of the function. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | The function as a lambda. Returns `null` when not found. |

**Example**

```kiwi
import "reflector"

fn greet(name: string): string
  "Hello, ${name}!"
end

# Retrieve the function by name and call it dynamically
fn_ref = reflector::getfunc("greet")

if fn_ref != null
  result = fn_ref.call(["Kiwi"])
  println result
  # => Hello, Kiwi!
end

# Returns null for unknown names
missing = reflector::getfunc("no_such_function")
println "Missing: ${missing}"
# => Missing: null
```

---

### `objcontext()`
Returns a hashmap representing the state of the program.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | Returns the current object reference. Returns `null` when not in an object context. |

**Example**

```kiwi
import "reflector"

struct Counter
  fn new(start: integer)
    @value = start
  end

  fn increment()
    @value += 1
    # Inside a method, objcontext() returns the current instance
    ctx = reflector::objcontext()
    println "Object context value: ${ctx.value}"
  end
end

c = Counter.new(10)
c.increment()
# => Object context value: 11

# Outside of a struct method, objcontext() returns null
println "Global context: ${reflector::objcontext()}"
# => Global context: null
```

---

### `retval()`
Returns the current stack frame return value.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | The return value at the top of the stack. |

**Example**

```kiwi
import "reflector"

fn compute(): integer
  return 42
  # After return is set, retval() would reflect it if called here
end

fn inspect_return(): string
  return "hello"
end

result = compute()
println "Returned: ${result}"
# => Returned: 42

# retval() is most useful inside a function body before the call exits,
# or in debug/trace hooks that inspect the active frame.
fn traced_add(a: integer, b: integer): integer
  sum = a + b
  return sum
end

println traced_add(3, 4)
# => 7
```

---

### `state()`
Returns a hashmap representing the state of the program.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap containing four keys: `packages` (a list containing names of packages imported), `structs` (a list containing names of structs defined), `functions` (a list containing names of functions defined), `stack` (a list of hashmaps containing frame variables, each with `variables` as a hashmap of variable names to values). |

**Example**

```kiwi
import "reflector"

fn helper(x: integer)
  x * 2
end

struct Point
  fn new(x, y)
    @x = x
    @y = y
  end
end

s = reflector::state()

println "Packages:  ${s.packages}"
println "Structs:   ${s.structs}"
println "Functions: ${s.functions}"

# Inspect variables in the current frame
frame_vars = s.stack[0].variables
println "Frame variables: ${frame_vars}"
```

---

## `Callable`

A struct representing a callable function or lambda, providing reflection capabilities such as invoking it and inspecting its parameters and return type.

### Constructor

Creates a new Callable instance.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | The name of the callable. |
| `lambda` | `func` | The lambda representing the callable. |

**Example**

```kiwi
import "reflector"

# Wrap a lambda directly in a Callable
adder = do (a: integer, b: integer): integer
  a + b
end

c = Callable.new("adder", adder)
println c.call([3, 7])
# => 10
```

---

### `from_function(func_name)`

Static method to create a Callable from a function name.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `func_name` | The name of the function. |

**Returns**

| Type | Description |
| :--- | :--- |
| `Callable` | A new Callable instance wrapping the function. |

**Example**

```kiwi
import "reflector"

fn square(n: integer): integer
  n * n
end

c = Callable.from_function("square")
println c.call([9])
# => 81

# from_function returns a Callable wrapping a no-op lambda when the function is not found
missing = Callable.from_function("undefined_fn")
println missing.call([])
# => null
```

---

### `call(params)`

Invokes the callable with a set of parameters and returns its value.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `params` | A list of parameters to pass to the callable. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | The return value of the callable after invocation. |

**Example**

```kiwi
import "reflector"

fn repeat_str(s: string, n: integer): string
  result = ""
  repeat n do
    result += s
  end
  result
end

c = Callable.from_function("repeat_str")
println c.call(["kiwi-", 3])
# => kiwi-kiwi-kiwi-

# Works equally well with an inline lambda
doubler = Callable.new("doubler", do (x: integer) => x * 2)
println doubler.call([21])
# => 42
```

---

### `parameters()`

Returns a list of hashmaps containing parameter information.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of hashmaps, each containing: `name` (a string representing the name of the parameter), `default_value` (the default value), `type` (if a type annotation was used, the type name as a string), `position` (the integer position of the parameter). |

**Example**

```kiwi
import "reflector"

fn compute(x: integer, y: float, label: string = "result"): string
  "${label}: ${x + y}"
end

c = Callable.from_function("compute")

for p in c.parameters() do
  println "param[${p.position}] ${p.name}: ${p.type} (default: ${p.default_value})"
end
# => param[0] x: integer (default: null)
# => param[1] y: float (default: null)
# => param[2] label: string (default: result)
```

---

### `returns()`

Returns the type name of the return type of the callable.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A string containing the type name of the return type. |

**Example**

```kiwi
import "reflector"

fn greet(name: string): string
  "Hello, ${name}!"
end

fn add(a: integer, b: integer): integer
  a + b
end

fn untyped(x)
  x
end

c1 = Callable.from_function("greet")
c2 = Callable.from_function("add")
c3 = Callable.from_function("untyped")

println "greet returns:   ${c1.returns()}"    # => string
println "add returns:     ${c2.returns()}"    # => integer
println "untyped returns: ${c3.returns()}"    # => (empty or null — no annotation)
```
