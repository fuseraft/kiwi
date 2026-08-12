# `hashable`

The `hashable` package provides the `Hashable` struct, a mixin base that any struct can inherit from to gain automatic hashmap serialization of its instance variables.

```kiwi
import "hashable"
```

---

## `Hashable` struct

When a struct inherits from `Hashable`, it receives a `to_hash()` method that iterates over all instance variable keys exposed by the object and returns them as a hashmap of name-value pairs.

The implementation uses the built-in `.keys()` and `.get(k)` intrinsics, so it reflects whatever instance variables are set at the time `to_hash()` is called.

### Inheritance

Use the `extends` keyword to inherit from `Hashable`:

```kiwi
import "hashable"

struct Person extends Hashable
  fn new(name, age)
    @name = name
    @age  = age
  end
end
```

---

## Methods

### `to_hash()`

Returns a hashmap containing all instance variables of the object as key-value pairs. The keys are the instance variable names (without the `@` prefix).

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap of instance variable names to their current values. |

**Example**

```kiwi
import "hashable"

struct Point extends Hashable
  fn new(x, y)
    @x = x
    @y = y
  end
end

p = Point.new(3, 7)
h = p.to_hash()
println h          # prints: {x: 3, y: 7}
println h["x"]     # prints: 3
println h["y"]     # prints: 7
```

---

## Complete Example

```kiwi
import "hashable"

struct Config extends Hashable
  fn new(host, port, debug)
    @host  = host
    @port  = port
    @debug = debug
  end
end

cfg = Config.new("localhost", 8080, true)
data = cfg.to_hash()

println data["host"]   # prints: localhost
println data["port"]   # prints: 8080
println data["debug"]  # prints: true

# Serialize the whole config as a hashmap for logging or storage
println data
# prints: {host: localhost, port: 8080, debug: true}
```

`to_hash()` is especially useful when you need to convert a struct instance into a plain hashmap for serialization, logging, or passing to APIs that expect hashmaps rather than struct instances.
